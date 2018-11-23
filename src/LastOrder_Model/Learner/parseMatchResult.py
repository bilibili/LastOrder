import zmq
import shutil
import os
import socket
import time
import json
import traceback
import zlib, pickle


local_ip = socket.gethostbyname(socket.gethostname())
server_ip = "127.0.0.1"
if local_ip.find("146.196.58.202") != -1:
    server_ip = "10.124.4.2"
if local_ip.find("172.16.35") != -1:
    server_ip = "172.16.35.11"
if local_ip.find("146.196.58.194") != -1:
    server_ip = "10.124.4.1"
context = zmq.Context()
log_server_sender = context.socket(zmq.PUSH)
log_server_sender.connect("tcp://%s:6555" % server_ip)

# key: game id, value: our_score, enemy_score, our_ip, opponent_name, finish_time, has_send
match_dict = {}
#ip : list
waiting_send_result = {}
result_path = "../StarcraftAITournamentManager-master/server/results.txt"


while True:
    try:
        del_keys = []
        for key, items in match_dict.items():
            if items["has_send"] == 1:
                continue
            if items["finish_time"] != 0 and time.time() - items["finish_time"] > 30 * 60:
                del_keys.append(key)
            if items["our_all_score"] == "unknown" or items["enemy_all_score"] == "unknown":
                continue
            if items["our_ip"] not in waiting_send_result:
                waiting_send_result[items["our_ip"]] = []
            waiting_send_result[items["our_ip"]].append(items)
            match_dict[key]["has_send"] = 1
        if len(del_keys) > 0:
            for d_k in del_keys:
                match_dict[d_k]["has_send"] = 1
            send_message = {"msg_type": "TM_result"}
            send_message["detailResultMiss"] = len(del_keys)
            p = pickle.dumps(send_message)
            log_server_sender.send(p)

        if len(waiting_send_result) >= 10:
            out_str = json.dumps(waiting_send_result)
            send_message = {"msg_type": "TM_result"}
            send_message["matchDetailResult"] = out_str
            p = pickle.dumps(send_message)
            log_server_sender.send(p)

            print("send result")
            waiting_send_result = {}

        if int(time.time()) % 5 == 0:
            if os.path.exists(result_path) == False:
                continue
            with open(result_path, 'r') as f:
                for line in f.readlines():
                    if not line:
                        continue
                    raw_list = line.strip().split(" ")
                    data_list = [i for i in raw_list if i != ""]

                    if len(data_list) < 26:
                        continue

                    game_id = data_list[0]
                    host_name = data_list[2]
                    away_name = data_list[3]
                    if data_list[5] == "true":
                        host_win = 1
                    else:
                        host_win = 0
                    if data_list[6] == "true":
                        host_crash = 1
                    else:
                        host_crash = 0
                    if data_list[7] == "true":
                        away_crash = 1
                    else:
                        away_crash = 0
                    if data_list[8] == "true":
                        match_timeout = 1
                    else:
                        match_timeout = 0

                    host_score = int(data_list[9])
                    away_score = int(data_list[10])
                    game_length = int(int(data_list[11]) / (60 * 24))
                    host_ip = data_list[20][1:]
                    away_ip = data_list[21][1:]
                    finish_time = int(time.mktime(time.strptime(data_list[23], '%Y%m%d_%H%M%S')))

                    host_all_score = data_list[24] #[i for i in data_list[24].split(",") if i != ""]
                    away_all_score = data_list[25] #[i for i in data_list[25].split(",") if i != ""]

                    if game_id in match_dict:
                        if match_dict[game_id]["our_all_score"] != "unknown" \
                        and match_dict[game_id]["enemy_all_score"] != "unknown":
                            continue
                    else:
                        match_dict[game_id] = {"our_score": 0,
                                               "enemy_score": 0,
                                               "our_ip": "",
                                               "opponent_name": "",
                                               "finish_time": 0,
                                               "game_length": 0,
                                               "our_win": -1,
                                               "enemy_crash": -1,
                                               "match_timeout": -1,
                                               "has_send": 0,
                                               "our_all_score": "unknown",
                                               "enemy_all_score": "unknown"}

                    if host_name == "LastOrder":
                        our_score = host_score
                        enemy_score = away_score
                        our_ip = host_ip
                        opponent_name = away_name
                        our_win = host_win
                        enemy_crash = away_crash
                    else:
                        our_score = away_score
                        enemy_score = host_score
                        our_ip = away_ip
                        opponent_name = host_name
                        our_win = 0 if host_win == 1 else 1
                        enemy_crash = host_crash
                        our_all_score = away_all_score
                        enemy_all_score = host_all_score

                        if our_all_score == "Error" or enemy_all_score == "Error":
                            continue

                    if match_dict[game_id]["our_score"] == 0:
                        match_dict[game_id]["our_score"] = our_score
                    if match_dict[game_id]["enemy_score"] == 0:
                        match_dict[game_id]["enemy_score"] = enemy_score
                    if match_dict[game_id]["our_ip"] == "":
                        match_dict[game_id]["our_ip"] = our_ip
                    if match_dict[game_id]["opponent_name"] == "":
                        match_dict[game_id]["opponent_name"] = opponent_name
                    if match_dict[game_id]["finish_time"] == 0:
                        match_dict[game_id]["finish_time"] = finish_time
                    if match_dict[game_id]["game_length"] == 0:
                        match_dict[game_id]["game_length"] = game_length
                    if match_dict[game_id]["our_win"] == -1:
                        match_dict[game_id]["our_win"] = our_win
                    if match_dict[game_id]["enemy_crash"] == -1:
                        match_dict[game_id]["enemy_crash"] = enemy_crash
                    if match_dict[game_id]["match_timeout"] == -1:
                        match_dict[game_id]["match_timeout"] = match_timeout
                    if match_dict[game_id]["our_all_score"] == "unknown":
                        if our_all_score != "unknown":
                            match_dict[game_id]["our_all_score"] = [int(i) for i in our_all_score.split(",") if i != ""]
                    if match_dict[game_id]["enemy_all_score"] == "unknown":
                        if enemy_all_score != "unknown":
                            match_dict[game_id]["enemy_all_score"] = [int(i) for i in enemy_all_score.split(",") if i != ""]

    except Exception:
        err_str = traceback.format_exc()
        with open('./log/parseMatch_error_log', 'a') as f:
            f.write(err_str + '\n')
        continue



