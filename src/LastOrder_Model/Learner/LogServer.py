import tensorflow as tf
import zmq
import shutil
import os
import socket
import logging
import time
import json
from Instance import Instance, Training_Instance, as_payload, Game
from SummaryLog import SummaryLog
import traceback
import math
from multiprocessing import Process, Queue, Pipe
import sys
import pickle



logging.basicConfig(filemode='a',
                    format='%(asctime)s,%(msecs)d,%(levelname)s::%(message)s',
                    datefmt='%H:%M:%S',
                    level=logging.INFO)


def log_save(log_str):
    log_instances = log_str.split('\n')
    info_instances = []
    error_instances = []
    critic_instances = []
    previous_list = info_instances
    for log in log_instances:
        if log == "":
            continue
        if len(log.split('::')) > 1:
            msg_type = log.split('::')[0].split(',')[1]
            if msg_type == "INFO":
                info_instances.append(log)
                previous_list = info_instances
            elif msg_type == "ERROR":
                error_instances.append(log)
                previous_list = error_instances
            elif msg_type == "CRITICAL":
                critic_instances.append(log)
                previous_list = critic_instances
        else:
            previous_list.append(log)

    #with open('./log/info_log', 'a') as f:
    #    for l in info_instances:
    #        f.write(l + '\n')
    with open('./log/error_log', 'a') as f:
        for l in error_instances:
            f.write(l + '\n')
    if len(critic_instances) > 0:
        header, match_info = critic_instances[0].split("CRITICAL::")
        frame, second, opponent, race, map_name, win, explore_rate = match_info.split("&")
        return float(win), float(explore_rate), opponent

        #with open('./log/critic_log', 'a') as f:
        #    header, match_info = critic_instances[0].split("CRITICAL::")
        #    frame, second, opponent, race, map_name, win, explore_rate = match_info.split("&")
        #    f.write(critic_instances[0] + '\n')
        #    return float(win), float(explore_rate), opponent
    else:
        return -999, 0, "1"


def setup_logger(name, log_file, level=logging.INFO):
    handler = logging.FileHandler(log_file)
    formatter = logging.Formatter('%(asctime)s,%(msecs)d,%(levelname)s::%(message)s')
    handler.setFormatter(formatter)
    logger = logging.getLogger(name)
    logger.setLevel(level)
    logger.addHandler(handler)
    return logger



def cal_score(our_score, enemy_score, time_frame):
    if our_score == 0 and enemy_score == 0:
        return 0
    else:
        time_dacay_parameter = int(time_frame / (24 * 60 * 5))
        scaled_score = (our_score - enemy_score) / 10000
        modified_score_diff = scaled_score * math.pow(0.7, time_dacay_parameter)
        #modified_score_diff = (our_score - enemy_score) / max(our_score, enemy_score)
        #modified_score_diff = max(-1, modified_score_diff)
        #modified_score_diff = min(1, modified_score_diff)
        return modified_score_diff


if __name__ == "__main__":
    if os.path.exists("./log"):
        shutil.rmtree('./log')
    if os.path.exists("./summary"):
        shutil.rmtree('./summary')
    if os.path.exists("./crash_log"):
        shutil.rmtree('./crash_log')
    if os.path.exists("./plt_image"):
        shutil.rmtree('./plt_image')
    if os.path.exists("./data_log"):
        shutil.rmtree('./data_log')
    os.makedirs('./data_log', exist_ok=True)
    os.makedirs('./plt_image', exist_ok=True)
    os.makedirs('./log', exist_ok=True)
    os.makedirs('./log/client', exist_ok=True)
    os.makedirs('./log/client/raw', exist_ok=True)
    os.makedirs('./summary', exist_ok=True)
    os.makedirs('./crash_log', exist_ok=True)

    #msg_queue = Queue()

    is_evaluation = False
    if len(sys.argv) == 2:
        if sys.argv[1] == 'evaluation':
            is_evaluation = True

    server_ip = "127.0.0.1"
    if socket.gethostbyname(socket.gethostname()).find("146.196.58.202") != -1:
        server_ip = "10.124.4.2"
    if socket.gethostbyname(socket.gethostname()).find("172.16.35") != -1:
        server_ip = "172.16.35.11"
    if socket.gethostbyname(socket.gethostname()).find("146.196.58.194") != -1:
        server_ip = "10.124.4.1"

    context = zmq.Context()
    #for waiting worker message
    receiver = context.socket(zmq.PULL)
    receiver.bind("tcp://%s:6555" %server_ip)

    poller = zmq.Poller()
    poller.register(receiver, zmq.POLLIN)

    #p = Process(target=parse_info, args=(msg_queue,is_evaluation))
    #p.start()

    context = zmq.Context()
    # Socket to send messages to model process
    build_sender = context.socket(zmq.PUSH)
    build_sender.connect("tcp://%s:7556" % server_ip)

    module_name = ["BuildingManager", "ProductionManager", "StrategyManager",
                   "Astar", "WorkerManager", "AttackManager", "Scout", "Main", "TimeManager"]

    summary_writer = tf.summary.FileWriter("./summary/log")
    summary_logger = SummaryLog(summary_writer)

    summary_logger.add_tag('low_explore/low_explore_rate_win_rate', 10, lambda x: 1 if x > 0 else 0)
    summary_logger.add_tag('low_explore/win_game_length', 10, lambda x: x)
    summary_logger.add_tag('low_explore/lose_game_length', 10, lambda x: x)
    summary_logger.add_tag('model/segment_win_rate', 10, lambda x: 1 if x > 0 else 0)
    summary_logger.add_tag('model/crash_game_count', 1, lambda x: x)
    summary_logger.add_tag('model/timeout_game_count', 1, lambda x: x)

    summary_logger.add_tag('latency/client_server_error', 1, lambda x: x)
    summary_logger.add_tag('latency/DLL_crash', 1, lambda x: x)
    summary_logger.add_tag('latency/client_timeout_match', 1, lambda x: x)
    summary_logger.add_tag('latency/client_avg_timeout_count', 10, lambda x: x)
    summary_logger.add_tag('latency/latency_too_large', 1, lambda x: x)
    summary_logger.add_tag('latency/update_too_long', 1, lambda x: x)
    summary_logger.add_tag('latency/model_update_too_long', 1, lambda x: x)
    summary_logger.add_tag('latency/recv_detail_result', 1, lambda x: x)
    summary_logger.add_tag('latency/recv_detail_result_miss', 1, lambda x: x)
    summary_logger.add_tag('latency/score_length_not_enough', 1, lambda x: x)

    multi_bots = True
    summary_logger.add_tag('low_explore/episode_discount_reward', 10, lambda x: x)
    summary_logger.add_tag('latency/sc_client_latency', 500, lambda x: x)
    summary_logger.add_tag('latency/actor_model_Update_Interval', 100, lambda x: x)
    summary_logger.add_tag('latency/no_instance_count', 1, lambda x: x)
    summary_logger.add_tag('latency/receive_miss_count', 1, lambda x: x)
    summary_logger.add_tag('latency/receive_instance_per_minutes', 1, lambda x: x)
    summary_logger.add_tag('latency/match_total_action_count', 10, lambda x: x)
    summary_logger.add_tag('latency/match_detail_result', 1, lambda x: x)
    summary_logger.add_tag('latency/detail_receive_miss_count', 1, lambda x: x)

    summary_logger.add_tag('latency/avg_remove_instance_count', 100, lambda x: x)
    summary_logger.add_tag('latency/max_match_action_count', 1, lambda x: x)
    summary_logger.add_tag('latency/avg_sampling_remove_instance_count', 100, lambda x: x)

    summary_logger.add_tag('evaluate/epoch_win_rate', 1, lambda x: x)
    summary_logger.add_tag('evaluate/all_win_rate', 1, lambda x: x)

    summary_logger.add_tag('latency/too_many_production_fail', 1, lambda x: x)
    evaluate_dict = {}
    evaluate_dict_index = {}
    evaluate_dict_num = []
    now_bot_num = 0
    evaluate_epoch = 1
    all_win_rate = []

    if is_evaluation:
        config_file_path = "../StarcraftAITournamentManager-master/server/server_settings.json"
        all_opponent_bots_name = []
        with open(config_file_path, 'r')as f:
            config_json = f.read()
            config_info = json.loads(config_json)
            for index, bot in enumerate(config_info["bots"]):
                if index == 0:
                    continue
                bot_name = "".join(list(filter(str.isalpha, bot["BotName"])))
                if bot_name not in all_opponent_bots_name:
                    all_opponent_bots_name.append(bot_name)

    max_action_count = 0
    receive_count_dict = 0
    ip_send_data_set = set()
    receive_instance_dict = []
    receive_instance_num = 0
    cal_receive_count_time = time.time()
    total_game_count = 0
    next_print_time = time.time()

    # ip : []
    detail_result_dict = {}
    instance_without_score_dict = []

    next_match_result_time = time.time()
    total_recv_detail = 0
    total_match_instance = 0

    while True:
        try:

            if time.time() > next_print_time:
                summary_writer.flush()
                print("differnet ip count: %d" % len(ip_send_data_set))
                if max_action_count != 0:
                    summary_logger.add_summary('latency/max_match_action_count', max_action_count)
                max_action_count = 0
                next_print_time = time.time() + 10

            if time.time() > cal_receive_count_time:
                summary_logger.add_summary('latency/receive_instance_per_minutes', receive_count_dict)
                receive_count_dict = 0
                cal_receive_count_time = time.time() + 120

            socks = dict(poller.poll(timeout=10))
            # get msg
            msg_count = 0
            instance_count = 0
            detail_count = 0
            start_recv_time = time.time()
            if receiver in socks and socks[receiver] == zmq.POLLIN:
                while True:
                    try:
                        data = receiver.recv(zmq.NOBLOCK)
                        msg_count += 1
                        msg_data = pickle.loads(data)
                        data_type = msg_data["msg_type"]
                        if data_type == "client_msg":
                            instance_count += 1
                            from_ip = msg_data["local_ip"]
                            opponent_name = msg_data["opponent_name"]
                            explore_rate = msg_data["explore_rate"]
                            map_name = msg_data["mapName"]
                            finish_time = msg_data["finish_time"]
                            del msg_data["local_ip"]
                            del msg_data["opponent_name"]
                            del msg_data["explore_rate"]
                            del msg_data["mapName"]
                            del msg_data["finish_time"]

                            if "InstanceUpdateAll" not in msg_data:
                                summary_logger.add_summary('latency/no_instance_count', 1)

                            for msg_type, receive_data in msg_data.items():
                                if msg_type == "InstanceUpdateAll":
                                    ip_send_data_set.add(from_ip)
                                    #training_data = json.loads(receive_data, object_hook=as_payload)
                                    # training_data = receive_data#pickle.loads(receive_data)
                                    training_data = receive_data.training_instances
                                    print("from %s, receive end log, explore rate %d, size %d, opponent %s" % (
                                        from_ip, int(explore_rate), len(training_data), opponent_name))
                                    if len(training_data) > max_action_count:
                                        max_action_count = len(training_data)

                                    instance_without_score_dict.append({"instance": training_data,
                                                                        "our_ip": from_ip,
                                                                        "opponent_name": opponent_name,
                                                                        "finish_time": finish_time,
                                                                        "explore_rate": int(explore_rate),
                                                                        "map_name": map_name,
                                                                        "game_features": receive_data.all_instances_features,
                                                                        "oppo_and_time": receive_data.oppo_and_time
                                                                        })

                                elif msg_type == "log":
                                    win, explore_rate, opponent = log_save(receive_data)
                                    if win == -999:
                                        print("from %s, receive error log" % (from_ip))
                                        continue
                                    total_game_count += 1

                                elif msg_type == "clientLatency":
                                    log_list = json.loads(receive_data)
                                    for i in log_list:
                                        summary_logger.add_summary("latency/sc_client_latency", i)

                                elif msg_type == "actorModelUpdateInterval":
                                    log_list = json.loads(receive_data)
                                    for i in log_list:
                                        summary_logger.add_summary("latency/actor_model_Update_Interval", i)

                                elif msg_type == "actions":
                                    summary_logger.add_summary('latency/match_total_action_count',
                                                               len(receive_data.split(',')))

                                elif msg_type == "clientlog":
                                    current_time = time.strftime('%Y%m%d_%H%M%S', time.localtime(time.time()))
                                    hasError = "BIG_ERROR_"
                                    #for m in module_name:
                                    #   if receive_data.find(hasError + m) != -1:
                                    #       if os.path.exists("./log/client/%s" % m) == False:
                                    #           os.makedirs("./log/client/%s" % m, exist_ok=True)
                                    #       fileName = "./log/client/%s/%s_%s" % (m, from_ip, current_time)
                                    #       with open(fileName, 'w')as f:
                                    #           f.write(receive_data)
                                    #fileName = "./log/client/raw/%s_%s" % (from_ip, current_time)
                                    #with open(fileName, 'w')as f:
                                    #   f.write(receive_data)

                                elif msg_type == "timeoutlog":
                                    if int(receive_data) > 300:
                                        summary_logger.add_summary('latency/client_timeout_match', 1)
                                    summary_logger.add_summary('latency/client_avg_timeout_count', int(receive_data))

                                elif msg_type == "latencyLarge":
                                    for latency in receive_data.split(","):
                                        summary_logger.add_summary('latency/latency_too_large', int(latency))

                                elif msg_type == "Updatelong":
                                    for latency in receive_data.split(","):
                                        summary_logger.add_summary('latency/update_too_long', int(latency))

                                elif msg_type == "modelUpdatelong":
                                    for latency in receive_data.split(","):
                                        summary_logger.add_summary('latency/model_update_too_long', int(latency))

                                elif msg_type == "removeInstanceCount":
                                    summary_logger.add_summary('latency/avg_remove_instance_count', int(receive_data))

                                elif msg_type == "samplingRemoveInstanceCount":
                                    summary_logger.add_summary('latency/avg_sampling_remove_instance_count',
                                                               int(receive_data))

                        elif data_type == "crash_dump":
                            raw_data = msg_data["dump"]
                            summary_logger.add_summary('latency/DLL_crash', 1)
                            file_name = "./crash_log/minidump_" + str(
                                summary_logger.get_tag_count('latency/DLL_crash')) + "_" + \
                                        time.strftime('%H%M%S', time.localtime(time.time())) + ".dmp"
                            with open(file_name, 'wb') as f:
                                f.write(raw_data)

                        elif data_type == "too_many_production_fail":
                            summary_logger.add_summary('latency/too_many_production_fail', 1)

                        elif data_type == "TM_result":
                            detail_count += 1
                            for msg_type, receive_data in msg_data.items():
                                if msg_type == "detailResultMiss":
                                    summary_logger.add_summary('latency/recv_detail_result_miss', 1)
                                elif msg_type == "matchDetailResult":
                                    result_data = json.loads(receive_data)
                                    for ip, detail_list in result_data.items():
                                        if ip not in detail_result_dict:
                                            detail_result_dict[ip] = []
                                        detail_result_dict[ip].extend(detail_list)
                                        total_recv_detail += len(detail_list)
                                        #for i in range(len(detail_list)):
                                        #    summary_logger.add_summary('latency/recv_detail_result', 1)
                                    summary_logger.add_summary('latency/recv_detail_result', total_recv_detail)
                    except zmq.ZMQError as e:
                        break
                if msg_count > 0:
                    print("process msg: %d, instance: %d, time: %f" %(msg_count, instance_count, time.time() - start_recv_time))

            if time.time() > next_match_result_time:
                match_result_start_time = time.time()
                next_match_result_time = time.time() + 60

                # client instance match detail result
                remove_instance_index = set()
                remove_detail_index = {}
                for instance_index, instance in enumerate(instance_without_score_dict):
                    instance_ip = instance["our_ip"]
                    if instance_ip not in detail_result_dict:
                        continue
                    for d_index, d_result in enumerate(detail_result_dict[instance_ip]):
                        if d_result["opponent_name"] == instance["opponent_name"] and abs(d_result["finish_time"] - instance["finish_time"]) < 70:
                            if d_result["our_score"] - d_result["enemy_score"] < 0:
                                modified_score_diff = (d_result["our_score"] - d_result["enemy_score"]) / max(
                                    d_result["our_score"], d_result["enemy_score"])
                            else:
                                max_value = max(d_result["our_score"], d_result["enemy_score"])
                                modified_score_diff = (d_result["our_score"] - d_result["enemy_score"]) / max_value

                            # enemy abnormal
                            if d_result["enemy_score"] < 1000:
                                if instance_ip not in remove_detail_index:
                                    remove_detail_index[instance_ip] = set()
                                remove_detail_index[instance_ip].add(d_index)
                                remove_instance_index.add(instance_index)
                                break

                            # opponent has bug
                            if modified_score_diff > 0.8:
                                if instance_ip not in remove_detail_index:
                                    remove_detail_index[instance_ip] = set()
                                remove_detail_index[instance_ip].add(d_index)
                                remove_instance_index.add(instance_index)
                                break

                            # has crash or timeout
                            if d_result["enemy_crash"] == 1 or d_result["match_timeout"] == 1:
                                if d_result["enemy_crash"] == 1:
                                    summary_logger.add_summary('model/crash_game_count', 1)
                                if d_result["match_timeout"] == 1:
                                    summary_logger.add_summary('model/timeout_game_count', 1)

                                # discard timeout match
                                #if d_result["match_timeout"] == 1:  # or d_result["enemy_crash"] == 1:
                                #    if instance_ip not in remove_detail_index:
                                #        remove_detail_index[instance_ip] = set()
                                #    remove_detail_index[instance_ip].add(d_index)
                                #    remove_instance_index.add(instance_index)
                                #    break
                                # discard match instance
                                if modified_score_diff < 0.2:
                                    if instance_ip not in remove_detail_index:
                                        remove_detail_index[instance_ip] = set()
                                    remove_detail_index[instance_ip].add(d_index)
                                    remove_instance_index.add(instance_index)
                                    break

                            # save win rate info
                            receive_count_dict += len(instance["instance"])
                            win = d_result["our_win"]
                            summary_logger.add_summary('model/segment_win_rate', win)
                            if instance["explore_rate"] <= 2:
                                summary_logger.add_summary('low_explore/low_explore_rate_win_rate', win)
                            if multi_bots:
                                opponent = "".join(list(filter(str.isalpha, instance["opponent_name"])))
                                # evaluate each epoch win rate
                                if is_evaluation:
                                    if opponent not in evaluate_dict:
                                        evaluate_dict[opponent] = []
                                        evaluate_dict_index[opponent] = now_bot_num
                                        evaluate_dict_num.append(0)
                                        now_bot_num += 1

                                    evaluate_dict[opponent].append(win)
                                    evaluate_dict_num[evaluate_dict_index[opponent]] += 1
                                    if evaluate_dict_num[evaluate_dict_index[opponent]] > evaluate_epoch:
                                        evaluate_dict_num[evaluate_dict_index[opponent]] -= 1
                                        evaluate_dict[opponent].pop(0)
                                    elif evaluate_dict_num[evaluate_dict_index[opponent]] == evaluate_epoch:
                                        if sum(evaluate_dict_num) == evaluate_epoch * len(all_opponent_bots_name):
                                            sum_win = 0
                                            for (opponent_name, win_list) in evaluate_dict.items():
                                                sum_win += sum(win_list)
                                                evaluate_dict[opponent_name] = []
                                                evaluate_dict_num[evaluate_dict_index[opponent_name]] = 0
                                            epoch_win_rate = sum_win / (evaluate_epoch * now_bot_num)
                                            all_win_rate.append(epoch_win_rate)
                                            summary_logger.add_summary('evaluate/epoch_win_rate', epoch_win_rate)
                                            summary_logger.add_summary('evaluate/all_win_rate',
                                                                       sum(all_win_rate) / len(all_win_rate))

                                title_segment = ("bots/win_rate_%s" % opponent)
                                if not summary_logger.has_tag(title_segment):
                                    summary_logger.add_tag(title_segment, 10, lambda x: 1 if x > 0 else 0)
                                summary_logger.add_summary(title_segment, win)
                                if instance["explore_rate"] <= 2:
                                    title_low_rate = ("bots/low_explore_win_rate_%s" % opponent)
                                    if not summary_logger.has_tag(title_low_rate):
                                        summary_logger.add_tag(title_low_rate, 10, lambda x: 1 if x > 0 else 0)
                                    summary_logger.add_summary(title_low_rate, win)
                            #map win rate
                            map_summary_name = "map/win_rate_%s" %instance["map_name"]
                            if summary_logger.has_tag(map_summary_name) == False:
                                summary_logger.add_tag(map_summary_name, 10, lambda x: 1 if x > 0 else 0)
                            summary_logger.add_summary(map_summary_name, win)

                            if instance["explore_rate"] <= 5:
                                if d_result["our_win"] == 1:
                                    summary_logger.add_summary("low_explore/win_game_length",
                                                               d_result["game_length"])
                                else:
                                    summary_logger.add_summary("low_explore/lose_game_length",
                                                               d_result["game_length"])

                            # cal score
                            modified_score_diff = (d_result["our_score"] - d_result["enemy_score"]) / max(
                                d_result["our_score"], d_result["enemy_score"])
                            time_decay = max(0, d_result["game_length"] - 10)
                            time_decay_parameter = math.pow(0.97, time_decay)
                            modified_score_diff = modified_score_diff * time_decay_parameter
                            modified_score_diff = max(-1, modified_score_diff)
                            modified_score_diff = min(0.5, modified_score_diff)

                            # lose
                            if d_result["our_win"] == 0:
                                final_score = modified_score_diff
                            else:
                                final_score = modified_score_diff

                            episode_reward = final_score *  math.pow(0.98,
                                                                    int(d_result["game_length"] / 2))

                            instance_item_index = len(instance["instance"]) - 1
                            temp_remove_index = set()
                            while instance_item_index >= 0:
                                if instance["instance"][instance_item_index].is_end == "1":
                                    instance["instance"][instance_item_index].terminal_reward = final_score
                                else:
                                    break

                                    #init_match_score_index = int(instance["instance"][instance_item_index].time / (10 * 24))
                                    #td_n_match_score_index = int(instance["instance"][instance_item_index].td_n_instance_time / (10 * 24))
                                    #if len(d_result["our_all_score"]) > td_n_match_score_index and len(d_result["enemy_all_score"]) > td_n_match_score_index:
                                    #    o_diff = d_result["our_all_score"][td_n_match_score_index] - d_result["our_all_score"][init_match_score_index]
                                    #    e_diff = d_result["enemy_all_score"][td_n_match_score_index] - d_result["enemy_all_score"][init_match_score_index]
                                    #    instance_reward = (o_diff - e_diff)/(max(o_diff, e_diff) * 50)
                                    #    time_decay = max(0, int(instance["instance"][instance_item_index].td_n_instance_time / (24 * 60)) - 10)
                                    #    time_decay_parameter = math.pow(0.97, time_decay)
                                    #    instance_reward = instance_reward * time_decay_parameter
                                    #    instance_reward = max(-0.01, instance_reward)
                                    #    instance_reward = min(0.01, instance_reward)
                                    #    instance["instance"][instance_item_index].instant_rewards = instance_reward
                                    #else:
                                    #    summary_logger.add_summary('latency/score_length_not_enough', 1)
                                    #    temp_remove_index.add(instance_item_index)

                                instance_item_index -= 1
                            instance["instance"] = [t_item for t_index, t_item in enumerate(instance["instance"]) if
                                                    t_index not in temp_remove_index]

                            now_game = Game(instance["game_features"], instance["instance"], instance["oppo_and_time"])
                            receive_instance_dict.append(now_game)
                            receive_instance_num += len(instance["instance"])
                            # receive_instance_dict.extend(instance["instance"])
                            if instance["explore_rate"] < 5:
                                summary_logger.add_summary("low_explore/episode_discount_reward",
                                                           episode_reward)
                            #
                            total_match_instance += 1

                            if instance_ip not in remove_detail_index:
                                remove_detail_index[instance_ip] = set()
                            remove_detail_index[instance_ip].add(d_index)
                            remove_instance_index.add(instance_index)
                            break

                    if time.time() - instance["finish_time"] > 30 * 60:
                        summary_logger.add_summary('latency/detail_receive_miss_count', 1)
                        with open('./log/detail_receive_miss_count', 'a') as f:
                            f.write(instance["our_ip"] + "_" + instance["opponent_name"]
                                    + "_" + str(instance["finish_time"]) + '\n')
                        remove_instance_index.add(instance_index)
                summary_logger.add_summary('latency/match_detail_result', total_match_instance)

                #log miss instance info
                for instance_ip, d_result_list in detail_result_dict.items():
                    for d_index, d_result in enumerate(d_result_list):
                        if time.time() - d_result["finish_time"] > 30 * 60:
                            summary_logger.add_summary('latency/receive_miss_count', 1)
                            with open('./log/receive_miss_count', 'a') as f:
                                f.write(json.dumps(d_result) + '\n')
                            if instance_ip not in remove_detail_index:
                                remove_detail_index[instance_ip] = set()
                            remove_detail_index[instance_ip].add(d_index)

                #remove instance
                instance_without_score_dict = [t_item for t_index, t_item in enumerate(instance_without_score_dict) if
                                                       t_index not in remove_instance_index]
                for ip, remove_index in remove_detail_index.items():
                    detail_result_dict[ip] = [d_item for d_index, d_item in enumerate(detail_result_dict[ip]) if d_index not in remove_index]

                #send to TainingServer
                if is_evaluation == False:
                    if receive_instance_num > 3000:
                        send_str = pickle.dumps(receive_instance_dict, protocol=pickle.HIGHEST_PROTOCOL)
                        receive_instance_num = 0
                        receive_instance_dict = []
                        build_sender.send(send_str)
                else:
                    receive_instance_dict = []
                print("match_result compute time: %f" %(time.time() - match_result_start_time))


        except Exception:
            err_str = traceback.format_exc()
            with open('./log/logserver_error_log', 'a') as f:
                f.write(err_str + '\n')
            continue



