import tensorflow as tf
import numpy as np
import zmq
from ACNet import ACNet
import shutil
import os
import socket
import logging
import time
import json
import sys
import random
from Instance import Instance, Training_Instance, as_payload, Game
from SumTree import ReplayBuffer
from SummaryLog import SummaryLog
import traceback
import copy
from multiprocessing import Process, Queue
from os import listdir
from os.path import isfile, join
import pickle



logging.basicConfig(filemode='a',
                    format='%(asctime)s,%(msecs)d,%(levelname)s::%(message)s',
                    datefmt='%H:%M:%S',
                    level=logging.INFO)
def setup_logger(name, log_file, level=logging.INFO):
    handler = logging.FileHandler(log_file)
    formatter = logging.Formatter('%(asctime)s,%(msecs)d,%(levelname)s::%(message)s')
    handler.setFormatter(formatter)
    logger = logging.getLogger(name)
    logger.setLevel(level)
    logger.addHandler(handler)
    return logger


def do_sgd(q_model, target_q_model, training_data,
           summary_logger, replay_buffer,
           training_instances_priority, model_type, game_index):
    td_n_state_max_action = q_model.get_target_max_q_value_action(training_data, True, game_index)
    target_q_value = target_q_model.get_target_q_value_estimation(training_data, td_n_state_max_action, game_index)

    updated_priority = q_model.sgd(training_data, target_q_value, summary_logger,
                                   training_instances_priority, replay_buffer, model_type, game_index)
    return updated_priority

    #return training_instances_priority



if __name__ == "__main__":
    model_type = sys.argv[1]
    if sys.argv[2] == "0":
        readmodel = False
    else:
        readmodel = True

    read_replay = False
    if len(sys.argv) == 4:
        if sys.argv[3] == "1":
            read_replay = True

    if os.path.exists("./model_log/%s"%model_type):
        shutil.rmtree('./model_log/%s'%model_type)
    if os.path.exists("./profile_log"):
        shutil.rmtree('./profile_log')
    if os.path.exists("./saved_model"):
        shutil.rmtree('./saved_model')
    os.makedirs('./profile_log', exist_ok=True)
    os.makedirs('./saved_model', exist_ok=True)

    os.makedirs('./model_log/%s'%model_type, exist_ok=True)
    os.makedirs('./summary/%s'%model_type, exist_ok=True)
    log_basic_info = setup_logger("basic", './model_log/%s/log'%model_type)

    server_ip = "127.0.0.1"
    if socket.gethostbyname(socket.gethostname()).find("146.196.58.202") != -1:
        server_ip = "10.124.4.2"
    if socket.gethostbyname(socket.gethostname()).find("172.16.35") != -1:
        server_ip = "172.16.35.11"
    if socket.gethostbyname(socket.gethostname()).find("146.196.58.194") != -1:
        server_ip = "10.124.4.1"

    context = zmq.Context()
    if model_type == "build":
        #waiting for logServer instance
        receiver = context.socket(zmq.PULL)
        receiver.bind("tcp://%s:7556" %server_ip)
        #sending updated model to worker
        sender = context.socket(zmq.PUB)
        sender.bind("tcp://%s:6556" %server_ip)

    poller = zmq.Poller()
    poller.register(receiver, zmq.POLLIN)

    global_ac = ACNet("build_q")
    target_global_ac = ACNet("build_target_q")

    e1_params = [t for t in tf.trainable_variables() if t.name.startswith(global_ac.scope)]
    e1_params = sorted(e1_params, key=lambda v: v.name)
    e2_params = [t for t in tf.trainable_variables() if t.name.startswith(target_global_ac.scope)]
    e2_params = sorted(e2_params, key=lambda v: v.name)
    update_ops = []
    for e1_v, e2_v in zip(e1_params, e2_params):
        op = tf.assign(e2_v, e1_v)
        update_ops.append(op)

    if readmodel:
        model_name = "q_model"
        for each_file in os.listdir("training_saved_model"):
            site = each_file.find(".index")
            if site != -1:
                model_name = each_file[0:site]
        reader = tf.train.NewCheckpointReader('training_saved_model/' + model_name)
        restore_dict = dict()
        for v in tf.trainable_variables():
            tensor_name = v.name.split(':')[0]
            if reader.has_tensor(tensor_name):
                restore_dict[tensor_name] = v
        training_saver = tf.train.Saver(restore_dict)

    saver = tf.train.Saver(max_to_keep=20)
    start_training_time = time.time()
    save_count = 0
    #config = tf.ConfigProto(inter_op_parallelism_threads=20,
    #                       intra_op_parallelism_threads=1)

    with tf.Session() as session:
        session.run(tf.global_variables_initializer())
        if readmodel:
            training_saver.restore(session, 'training_saved_model/' + model_name)
        session.graph.finalize()

        summary_writer = tf.summary.FileWriter("./summary/model_%s" % model_type, session.graph)
        summary_logger = SummaryLog(summary_writer)

        summary_logger.add_tag('latency/sgd_round', 100, lambda x: x)
        summary_logger.add_tag('latency/duplicate_instance', 1, lambda x: x)
        summary_logger.add_tag('model/first_priority', 500, lambda x: x)
        summary_logger.add_tag('model/q_value_loss', 500, lambda x: x)
        summary_logger.add_tag('model/avg_td_error', 500, lambda x: x)
        summary_logger.add_tag('model/avg_IS', 500, lambda x: x)
        summary_logger.add_tag('model/q_model_global_norm', 500, lambda x: x)
        summary_logger.add_tag('model/q_model_q_value', 500, lambda x: x)
        summary_logger.add_tag('model/target_q_value', 500, lambda x: x)
        summary_logger.add_tag('low_explore/max_q_value', 200, lambda x: x)

        replay_buffer_size = 1000000
        replay_buffer = ReplayBuffer(replay_buffer_size, global_ac.a_index_name_dict, summary_logger, model_type)
        next_sync_model_time = 0
        sgd_time_budget = 2
        sync_target_model_time = 0
        mini_batch_size = 192
        game_index = {}
        now_index = 0

        if read_replay:
            replay_file = 'replay_buffer_instance.pickle'
            game_file = 'replay_buffer_game.pickle'
            with open(game_file, 'rb')as f:
                game_instances = pickle.load(f)
                for g in game_instances:
                    game_index[g.oppo_and_time] = g
                now_index = max([g.oppo_and_time for g in game_instances]) + 1
            with open(replay_file, 'rb')as f:
                replay_instances = pickle.load(f)
                for i in replay_instances:
                    i.insert_pool_time = time.time()
                    if i.oppo_and_time not in game_index:
                        print("not has game instance!!")
                        continue
                    replay_buffer.add(i.priority, i, i.opponent_name, False)
            print("read_data_finish !!!")
            #replay_buffer.data_count = 0

        try:
            while True:
                socks = dict(poller.poll(timeout=1))

                if time.time() - start_training_time > 60 * 10:
                    saver.save(session, "./saved_model/q_model", global_step=save_count, write_meta_graph=False)
                    save_count += 1
                    start_training_time = time.time()

                if int(time.time()) % 10 == 0:
                    summary_writer.flush()
                if int(time.time()) > next_sync_model_time:
                    serialized_str = global_ac.serializing()
                    print("sync model")
                    sender.send(serialized_str)
                    next_sync_model_time = time.time() + 10

                if int(time.time()) > sync_target_model_time:
                    tf.get_default_session().run(update_ops)
                    sync_target_model_time = int(time.time()) + 60 * 10
                    # log_basic_info.info("update target model")
                    print("update target model")

                if receiver in socks and socks[receiver] == zmq.POLLIN:
                    raw_data_list = []
                    while True:
                        try:
                            data = receiver.recv(zmq.NOBLOCK)
                            raw_data_list.append(data)
                        except zmq.ZMQError as e:
                            break

                    for raw_data in raw_data_list:
                        training_data = pickle.loads(raw_data)
                        if training_data == "save_replay_buffer":
                            replay_buffer.save_replay_buffer(game_index)
                            sys.exit()

                        for j in training_data:
                            # if game_index.__contains__(j.oppo_and_time):
                            if game_index.__contains__(now_index):
                                summary_logger.add_summary('latency/duplicate_instance', 1)
                                continue

                            this_game = Game(np.array(j.all_instances_features, dtype=np.int16), [], j.oppo_and_time)
                            this_game.oppo_and_time = now_index
                            game_index[now_index] = this_game

                            for i in j.training_instances:
                                i.oppo_and_time = now_index
                                if i.explore_rate <= 5:
                                    summary_logger.add_summary('low_explore/max_q_value', i.Q_reward)
                                i.trans_nparray()
                                i.insert_pool_time = time.time()
                                replay_buffer.add(i.priority, i, i.opponent_name, True)
                                summary_logger.add_summary('model/first_priority', i.priority)

                            now_index += 1

                        # for i in training_data:
                        #     if i.explore_rate <= 5:
                        #         summary_logger.add_summary('low_explore/max_q_value', i.Q_reward)
                        #     i.trans_nparray()
                        #     i.insert_pool_time = time.time()
                        #     replay_buffer.add(i.priority, i, i.opponent_name)
                        #     summary_logger.add_summary('model/first_priority', i.priority)

                if model_type == "build":
                    replay_buffer_min_count = 10000
                replay_buffer.print_debug_info()
                # guarantee data in pool do not related with each other
                if replay_buffer.get_data_count() >= replay_buffer_min_count:
                    start_time = time.time()
                    sgd_round = 0

                    while time.time() - start_time < sgd_time_budget:
                        sgd_round += 1
                        total_priority = replay_buffer.total_priority()
                        batch_instance = []
                        for index in range(mini_batch_size):
                            sample_priority = np.random.randint(1, total_priority)
                            tree_index, priority, data = replay_buffer.get(sample_priority)
                            # if isinstance(data, Training_Instance):
                            batch_instance.append([tree_index, data, sample_priority, priority])
                            # log_basic_info.info("tree index:%d" %(tree_index))

                        new_priority = do_sgd(global_ac, target_global_ac, [i[1] for i in batch_instance],
                                              summary_logger, replay_buffer,
                                              [i[-1] for i in batch_instance], model_type, game_index)
                        # update sample's priority
                        for index, sample in enumerate(batch_instance):
                            replay_buffer.update(sample[0], new_priority[index])

                    # log_basic_info.info("%s gradient update end, sgd round: %d" %(model_type, sgd_round))
                    print("%s gradient update end, sgd round: %d" % (model_type, sgd_round))
                    summary_logger.add_summary('latency/sgd_round', sgd_round)

        except Exception:
            err_str = traceback.format_exc()
            with open('./log/TrainServer_exec_%s_error_log' % model_type, 'a') as f:
                f.write(err_str + '\n')


