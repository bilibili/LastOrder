import time
import zmq
import sys
from ACNet import ACNet
import tensorflow as tf
import numpy as np
import logging
from io import StringIO
import socket
from Worker import Worker
import os
import subprocess
import ctypes
import traceback
import gc
import pickle, zlib
import json


def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False


if __name__ == "__main__":
    try:
        #if is_admin() == 0:
        #    #run as administrator
        #    ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, __file__, None, 1)
        #    sys.exit()

        build_logger = logging.getLogger("build")
        build_logger.setLevel(logging.DEBUG)
        build_log_stream = StringIO()
        b = logging.StreamHandler(build_log_stream)
        b.setFormatter(logging.Formatter("%(asctime)s.%(msecs)d,%(levelname)s::%(message)s",
                                         "%H:%M:%S"))
        build_logger.addHandler(b)

        attack_logger = logging.getLogger("attack")
        attack_logger.setLevel(logging.DEBUG)
        attack_log_stream = StringIO()
        a = logging.StreamHandler(attack_log_stream)
        a.setFormatter(logging.Formatter("%(asctime)s.%(msecs)d,%(levelname)s::%(message)s",
                                         "%H:%M:%S"))
        attack_logger.addHandler(a)

        local_ip = socket.gethostbyname(socket.gethostname())
        running_mode = "training"

        context = zmq.Context()
        client_ac = ACNet("build_q")
        client_ac.logging = build_logger
        client_ac.ip = local_ip

        # Socket to receive messages from dll
        receiver = context.socket(zmq.PULL)
        receiver.bind("tcp://127.0.0.1:5555")
        # Socket to send messages to dll
        sender = context.socket(zmq.PUSH)
        sender.bind("tcp://127.0.0.1:5556")

        poller = zmq.Poller()
        poller.register(receiver, zmq.POLLIN)

        readmodel = False
        if len(sys.argv) == 2:
            if sys.argv[1] == 'readmodel':
                readmodel = True
                model_name = "q_model"
                for each_file in os.listdir("saved_model"):
                    site = each_file.find(".index")
                    if site != -1:
                        model_name = each_file[0:site]
                reader = tf.train.NewCheckpointReader('saved_model/' + model_name)
                restore_dict = dict()
                for v in tf.trainable_variables():
                    tensor_name = v.name.split(':')[0]
                    if reader.has_tensor(tensor_name):
                        restore_dict[tensor_name] = v
                saver = tf.train.Saver(restore_dict)

        local_worker = Worker(client_ac, running_mode, build_logger, local_ip, sender, "build", readmodel)
        if readmodel == False:
            server_ip = "127.0.0.1"
            if local_ip.find("172.16.35") != -1:
                server_ip = "172.16.35.11"
            if local_ip.find("10.124") != -1:
                if local_ip.split('.')[2]=='3' and int(local_ip.split('.')[3])>205:
                    server_ip = "10.124.4.1"
                else:
                    server_ip = "10.124.4.2"

            training_server_sender = context.socket(zmq.PUSH)
            training_server_sender.connect("tcp://%s:6555" % server_ip)

            training_server_receiver = context.socket(zmq.SUB)
            training_server_receiver.connect("tcp://%s:6556" % server_ip)
            training_server_receiver.setsockopt_string(zmq.SUBSCRIBE, "")
            poller.register(training_server_receiver, zmq.POLLIN)
            client_ac.training_init(training_server_sender, training_server_receiver)
            local_worker.set_training_server_sender(training_server_sender)

        minidump_path = "C:/starcraft_minidump.dmp"
        #if os.path.exists(minidump_path):
            #os.remove(minidump_path)
        latest_update_time = time.time()

        send_result = True
        with tf.Session() as session:
            session.run(tf.global_variables_initializer())

            if readmodel:
                saver.restore(session, 'saved_model/'+model_name)

            while True:
                socks = dict(poller.poll(timeout=10))
                start_time = time.time()
                #for model weight update
                if readmodel == False:
                    if client_ac.receiver in socks and socks[client_ac.receiver] == zmq.POLLIN:
                        local_worker.update_model()
                if time.time() - start_time > 10:
                    local_worker.Updatelong.append(int(time.time() - start_time))
                    # training_server_sender.send_string(
                    #     "Updatelong_%s_all|||||%d" % (local_worker.local_ip, int(time.time() - start_time)))

                #get game client msg
                if receiver in socks and socks[receiver] == zmq.POLLIN:
                    raw_data_list = []
                    while True:
                        try:
                            data = receiver.recv_string(zmq.NOBLOCK)
                            raw_data_list.append(data)
                        except zmq.ZMQError as e:
                            break

                    start_time = time.time()
                    game_end = False
                    for receive_data in raw_data_list:
                        message_type, message_data = receive_data.split('|||')
                        if message_type == "build_model_request":
                            res = local_worker.get_predict_action(message_data)
                            if res == -1:
                                game_end = True

                        elif message_type == "update_state":
                            local_worker.update_state(message_data)

                        elif message_type == "log_latency":
                            local_worker.add_log_lantency(int(message_data))
                            if int(message_data) > 10:
                                local_worker.latencyLarge.append(message_data)

                        elif message_type == "log_timeout":
                            local_worker.timeoutlog.append(message_data)

                        elif message_type == "end_game":
                            if readmodel == False:
                                send_message = {"msg_type": "too_many_production_fail"}
                                p = pickle.dumps(send_message)
                                training_server_sender.send(p)
                                game_end = True
                                send_result = False

                    if time.time() - start_time > 10:
                        local_worker.modelUpdatelong.append(int(time.time() - start_time))
                        # training_server_sender.send_string(
                        #     "modelUpdatelong_%s_all|||||%d" % (local_worker.local_ip, int(time.time() - start_time)))

                    if game_end == True:
                        break

                #dll crash
                if os.path.exists(minidump_path):
                    if readmodel == False:
                        modify_time = os.path.getmtime(minidump_path)
                        if time.time() - modify_time > 1:
                            with open(minidump_path, 'rb') as f:
                                send_message = {"msg_type": "crash_dump"}
                                mini_dump = f.read()
                                send_message["dump"] = mini_dump
                                p = pickle.dumps(send_message)
                                training_server_sender.send(p)
                            os.remove(minidump_path)
                            #only break when current game crash
                            if time.time() - modify_time < 5:
                                break

            if readmodel == False:
                local_worker.send_message["log"] = build_log_stream.getvalue()
                if os.path.exists("../write/log_detail_file"):
                    with open('../write/log_detail_file', 'r') as f:
                        all_log_str = f.read()
                        local_worker.send_message["clientlog"] = all_log_str
                if send_result:
                    local_worker.game_end_send_message()


    except Exception:
        err_str = traceback.format_exc()
        #with open('../../serverMainError', 'w') as f:
            #f.write(err_str)
        print(err_str)

        if readmodel == False:
            build_logger.error('%s Server fail, %s' % (local_ip, err_str))
            local_worker.send_message["log"] = build_log_stream.getvalue()
            local_worker.game_end_send_message()


    time.sleep(1)
    receiver.close()
    sender.close()
    if readmodel == False:
        client_ac.training_socket_close()
    context.destroy()
    sys.exit()










