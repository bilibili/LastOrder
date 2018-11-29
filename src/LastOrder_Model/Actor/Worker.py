import numpy as np
import math
import json
import time
import zmq
import copy
from Instance import Instance, Training_Instance, Game
import pickle



class Worker:

    def __init__(self, AC_net, running_mode, logging_handle, local_ip, client_sender, worker_type, is_evaluation):
        self.worker_type = worker_type
        self.raw_instance_list = []
        #because bootstrap Q value estimation mainly based on state, but current action execution interval is different,
        # so set the max time interval as TD n's max step
        if self.worker_type == "build":
            self.max_TD_interval = 10
        elif self.worker_type == "attack":
            self.max_TD_interval = 1

        self.discount_rate = 0.98
        self.clipping_reward_threshold = 0.05
        self.previous_state_value = -999

        self.model = AC_net
        self.running_mode = running_mode
        self.first_time = 0
        self.log_handle = logging_handle
        self.local_ip = local_ip
        if local_ip.find("172.16.35") != -1:
            total_actor_count = 50
            ip_postfix = int(local_ip.split('.')[3]) - 11
            if ip_postfix <= 8:
                self.explore_rate = 5
            else:
                self.explore_rate = int(0.4 * (ip_postfix / total_actor_count) * 100)
        elif local_ip.find("10.124") != -1:
            if local_ip.split('.')[2]=='3' and int(local_ip.split('.')[3])>205:
                total_actor_count = 50
                ip_postfix = int(local_ip.split('.')[3]) - 205
                if ip_postfix <= 8:
                    self.explore_rate = 4
                else:
                    self.explore_rate = int(0.4 * (ip_postfix / total_actor_count) * 100)
            else:
                total_actor_count = 1000
                ip_postfix = int(local_ip.split('.')[2]) * 256 + int(local_ip.split('.')[3]) - 26
                self.explore_rate = int(math.pow(0.4, 1 + (ip_postfix / total_actor_count) * 7) * 100)
                #if ip_postfix % 2 == 0:
                    #self.explore_rate = max(self.explore_rate, 1)
        else:
            self.explore_rate = 50

        self.is_evaluation = is_evaluation
        if is_evaluation == True:
            self.explore_rate = 0

        self.client_sender = client_sender
        self.choose_action_list = []
        self.local_training_instance = []
        self.all_local_training_instance = []
        self.send_count = 1

        self.latency_data = []
        self.previous_model_update_time = 0
        self.model_update_interval_list = []
        self.accumulated_reward_list = []

        #self.max_time_decay_parameter = 0
        self.lstm_max_interval = 20 * 24
        self.max_time_decay_parameter = 0.0
        self.check_unique_interval = 3 * 24

        self.sampling_ration = [100 for i in range(61)]
        for i in range(61):
            if i < 20:
                self.sampling_ration[i] = 100
            elif i >= 20 and i < 30:
                self.sampling_ration[i] = 100
            elif i >= 30 and i < 40:
                self.sampling_ration[i] = 100
            elif i >= 40:
                self.sampling_ration[i] = 100

        self.send_message = {}
        self.mean_model_update_interval_list = []   # float
        self.mean_latency_data = [] # float
        self.Updatelong = []        # int
        self.latencyLarge = []      # string
        self.timeoutlog = []        # string
        self.modelUpdatelong = []   # int

        self.game_now_index = 0
        self.oppo_and_time = ""
        # self.used_index = set()
        self.game_index = {}


    def test(self):
        f = open('./serializedString_Jormungand Brood', 'r')
        info = f.read()
        for m in info.split('&&'):
            m = m.strip()
            self.get_predict_action(m)


    def set_training_server_sender(self, training_server_sender):
        self.training_server_sender = training_server_sender


    def update_state(self, receive_data):
        receive_data = receive_data.split("|")
        self.get_message_data(receive_data, False)


    #intput format: model_type, feature, instant_reward, terminal_score, action_valid_info, is_game_end, attack_info, extra_info
    def get_message_data(self, field_info, is_instance):
        # feature, action_valid_info format:  name: value \n ...
        extra_info = field_info[-1]
        frame_count, play_length, opponent, race, map_name = extra_info.split('&')
        opponent = opponent.replace(' ', '')
        opponent_for_oppotime = "".join(list(opponent)) + str(self.local_ip)
        opponent = "".join(list(filter(str.isalpha, opponent)))

        feature = field_info[1]
        terminal_score = float(field_info[3])
        instant_score = float(field_info[2])
        attack_info = field_info[6].split('&')

        action_valid_info = []
        for index, i in enumerate(field_info[4].split('\n')):
            if i == "":
                continue
            if i.split(':')[1] == "1":
                action_valid_info.append(1)
            elif i.split(':')[1] == "0":
                action_valid_info.append(0)
            else:
                continue
        origin_features = [float(i.split(':')[1]) for i in feature.split('\n') if i != ""]
        #opponent_name_feature = [0 for key, value in config.opponent_name_index_dict.items()]
        #only use opponent name feature in training
        #if self.is_evaluation == False:
        #    if opponent in config.opponent_name_index_dict:
        #        opponent_name_feature[config.opponent_name_index_dict[opponent]] = 1
        #origin_features.extend(opponent_name_feature)
        total_reward = instant_score

        # if is_instance == False:
        # insert whether message is instance or not
        if self.oppo_and_time == '':  # Get first message
            # assert len(self.game_index) == 0
            self.oppo_and_time = opponent_for_oppotime + "_" + str(time.time()) #time.strftime("%H%I%S", time.localtime(time.time()))
            # self.model.game[self.oppo_and_time] = np.array([[0 for i in range(self.model.feature_input_count)]])
            this_game = Game(np.array([[0 for i in range(self.model.feature_input_count)]]), [], self.oppo_and_time)
            self.game_index[self.oppo_and_time] = this_game
            self.game_index[self.oppo_and_time].instance_timestamp.append(-1)
            self.game_now_index = 0

        self.game_index[self.oppo_and_time].all_instances_features = np.row_stack(
            [self.game_index[self.oppo_and_time].all_instances_features, np.array([origin_features])]
        )
        self.game_index[self.oppo_and_time].instance_timestamp.append(int(frame_count))
        self.game_now_index += 1
        time_stamp_list = self.game_index[self.oppo_and_time].instance_timestamp

        cur_lstm_instance_list = [self.game_now_index]
        cur_time = int(frame_count)
        i = len(time_stamp_list) - 1
        while i >= 1 and len(cur_lstm_instance_list) < self.model.lstm_max_length:
            if cur_time - time_stamp_list[i] > self.lstm_max_interval:
                cur_lstm_instance_list.append(i)
                cur_time = time_stamp_list[i]
            i -= 1
        while len(cur_lstm_instance_list) < self.model.lstm_max_length:
            cur_lstm_instance_list.append(0)
        cur_lstm_instance_list = list(reversed(cur_lstm_instance_list))

        new_instance = Instance(int(frame_count), cur_lstm_instance_list,
                                "", "", total_reward, terminal_score,
                                field_info[5], action_valid_info, attack_info)

        return new_instance


    def check_attack_end(self, action_name):
        if action_name.find("AllInAttack") != -1:
            return self.raw_instance_list[-1].attack_info[1] == '0'
        if action_name.find("MutaliskHarass") != -1:
            return self.raw_instance_list[-1].attack_info[0] == '0'


    def check_sequnce(self, game_end):
        if game_end == 1:
            for index, item in enumerate(self.raw_instance_list[:-1]):
                self.send_instances(index, game_end)
        else:
            if len(self.raw_instance_list) == 1:
                return
            remove_index_list = []
            cur_time = self.raw_instance_list[-1].time
            for index, item in enumerate(self.raw_instance_list):
               if cur_time - self.raw_instance_list[index].time > 2 * 60 * 24:
                   self.send_instances(index, game_end)
                   remove_index_list.append(index)
               else:
                   break
            self.raw_instance_list = [item for index, item in enumerate(self.raw_instance_list)
                                      if index not in remove_index_list]


    def send_instances(self, start, game_end):
        td_discount = self.discount_rate
        terminal_state = copy.deepcopy(self.raw_instance_list[-1].state_feature)
        terminal_action_valid_info = self.raw_instance_list[-1].action_valid_info
        terminal_reward = self.raw_instance_list[-1].terminal_reward
        is_end = self.raw_instance_list[-1].is_end
        #step cost
        actual_reward = 0
        instant_rewards = actual_reward #+ diff_score

        if instant_rewards > self.clipping_reward_threshold:
            instant_rewards = self.clipping_reward_threshold
        if instant_rewards < -self.clipping_reward_threshold:
            instant_rewards = -self.clipping_reward_threshold

        max_Q_value = self.model.get_Q_value(self.raw_instance_list[start], self.oppo_and_time, self.game_index)
        training_instance = Training_Instance(self.raw_instance_list[start].time, self.raw_instance_list[start].state_feature, self.raw_instance_list[start].choose_action,
                                              td_discount, terminal_state, terminal_action_valid_info, terminal_reward, is_end, instant_rewards, 0,
                                              float(max_Q_value), self.explore_rate, self.opponent_name, self.oppo_and_time)

        # td_n_state_max_action = self.model.get_target_max_q_value_action([training_instance], False)
        # target_value = self.model.get_target_q_value_estimation([training_instance], td_n_state_max_action)
        # priority = self.model.get_priority([training_instance], target_value)

        # copy a new training_instance which has nparray
        training_instance_copy = copy.deepcopy(training_instance)
        training_instance_copy.trans_nparray()

        td_n_state_max_action = self.model.get_target_max_q_value_action([training_instance_copy], False, self.game_index)
        target_value = self.model.get_target_q_value_estimation([training_instance_copy], td_n_state_max_action, self.game_index)
        priority = self.model.get_priority([training_instance_copy], target_value, self.game_index)
        training_instance.priority = priority[0]
        self.local_training_instance.append(training_instance)
        self.all_local_training_instance.append(training_instance)

        if game_end == 1 and start == len(self.raw_instance_list) - 2:
            #remove redundant instance
            result_list = []
            start_frame = 0
            unique_action_set = set()
            unique_instance_list = []
            remove_count = 0
            remove_detail = []
            for index, instance in enumerate(self.all_local_training_instance):
                if instance.time - start_frame > self.check_unique_interval:
                    for unique_instance in unique_instance_list:
                        result_list.append(unique_instance)
                    start_frame = instance.time
                    unique_action_set = set()
                    unique_instance_list = []
                    unique_action_set.add(instance.choose_action)
                    unique_instance_list.append(instance)
                else:
                    if instance.choose_action not in unique_action_set:
                        unique_action_set.add(instance.choose_action)
                        unique_instance_list.append(instance)
                    else:
                        remove_count += 1
                        remove_detail.append([instance.choose_action, instance.time])
            for unique_instance in unique_instance_list:
                result_list.append(unique_instance)

            #sampling instance
            sampling_result_list = []
            sampling_remove_count = 0
            for instance in result_list:
                instance_frame_minute = int(instance.time / (24 * 60))
                if instance_frame_minute < len(self.sampling_ration):
                    sampling_ration = self.sampling_ration[instance_frame_minute]
                else:
                    sampling_ration = 0
                sampling_prob = np.random.randint(0, 100)
                if sampling_prob < sampling_ration:
                    sampling_result_list.append(instance)
                else:
                    sampling_remove_count += 1

            self.all_local_training_instance = sampling_result_list
            self.send_message["removeInstanceCount"] = remove_count
            self.send_message["samplingRemoveInstanceCount"] = sampling_remove_count
            # self.send_message["InstanceUpdateAll"] = self.all_local_training_instance #pickle.dumps(self.all_local_training_instance, protocol=pickle.HIGHEST_PROTOCOL)

            used_index = [0 for i in range(self.game_now_index + 1)]
            for inst in sampling_result_list:
                for f in inst.state_feature:
                    used_index[f] = 1
                for f in inst.terminal_state:
                    used_index[f] = 1

            minus = 0
            minus_index = []
            for i in range(len(used_index)):
                if used_index[i] == 0:
                    minus += 1
                minus_index.append(i-minus)

            for i in range(len(sampling_result_list)):
                for j in range(len(sampling_result_list[i].state_feature)):
                    sampling_result_list[i].state_feature[j] = minus_index[sampling_result_list[i].state_feature[j]]
                for j in range(len(sampling_result_list[i].terminal_state)):
                    sampling_result_list[i].terminal_state[j] = minus_index[sampling_result_list[i].terminal_state[j]]

            list_game = self.game_index[self.oppo_and_time].all_instances_features.tolist()
            pop_set = set()
            for index, i in enumerate(used_index):
                if i == 0:
                    pop_set.add(index)
            list_game = [t_item for t_index, t_item in enumerate(list_game) if
                                                    t_index not in pop_set]

            now_game = Game(list_game, sampling_result_list, self.oppo_and_time)
            self.send_message["InstanceUpdateAll"] = now_game


    def update_model(self):
        self.model.async_update()
        if self.previous_model_update_time != 0:
            interval = time.time() - self.previous_model_update_time
            self.model_update_interval_list.append(interval)
            self.previous_model_update_time = time.time()
            if len(self.model_update_interval_list) >= 10:
                self.mean_model_update_interval_list.append(sum(self.model_update_interval_list)/len(self.model_update_interval_list))
                #json_str = json.dumps(self.model_update_interval_list)
                # self.training_server_sender.send_string("actorModelUpdateInterval_%s_%s|||||%s" % (self.local_ip, self.worker_type, json_str))
                self.model_update_interval_list = []
        else:
            self.previous_model_update_time = time.time()


    def add_log_lantency(self, latency):
        self.latency_data.append(latency)
        if len(self.latency_data) >= 10:
            self.mean_latency_data.append(sum(self.latency_data)/len(self.latency_data))
            # json_str = json.dumps(self.latency_data)
            # self.training_server_sender.send_string("clientLatency_%s_%s|||||%s" %(self.local_ip, self.worker_type, json_str))
            self.latency_data = []


    def get_predict_action(self, receive_data):
        receive_data = receive_data.split("|")
        instance_data = self.get_message_data(receive_data, True)
        cur_frame, cur_time, opponent_name, opponent_race, map_name = receive_data[-1].split("&")
        self.opponent_name = opponent_name
        self.cur_time = cur_time
        self.send_message["mapName"] = map_name
        # not the terminal state
        if instance_data.is_end == "0":
            start_time = time.time()
            #self.log_handle.info("%s model begin" % (self.local_ip))
            choose_action = self.model.predict(instance_data, self.explore_rate, self.oppo_and_time, self.game_index)
            # if time.time() - start_time > 10:
            #     self.training_server_sender.send_string(
            #         "Worker1_%s_%s|||||%d" % (
            #         self.local_ip, self.worker_type, time.time() - start_time))

            start_time = time.time()
            # no valid action
            if choose_action == -1:
                back_str = self.worker_type + "|" + str(choose_action) + "|" + receive_data[-1]
                self.client_sender.send_string(back_str)
                return
            # if time.time() - start_time > 10:
            #     self.training_server_sender.send_string(
            #         "Worker2_%s_%s|||||%d" % (
            #         self.local_ip, self.worker_type, time.time() - start_time))

            # start_time = time.time()
            instance_data.choose_action = choose_action
            self.choose_action_list.append(choose_action)
            self.raw_instance_list.append(instance_data)
            self.check_sequnce(0)

            # start_time = time.time()
            back_str = self.worker_type + "|" + str(choose_action) + "|" + receive_data[-1]
            self.client_sender.send_string(back_str)
            #self.log_handle.info("%s model end" % (self.local_ip))
            # if time.time() - start_time > 10:
            #     self.training_server_sender.send_string(
            #         "Worker4_%s_%s|||||%d" % (
            #         self.local_ip, self.worker_type, time.time() - start_time))
            return 0

        else:
            #self.log_handle.info("%s game end message" % self.local_ip)
            if self.running_mode == "training":
                log_str = receive_data[-1] + "&" + str(instance_data.terminal_reward) + "&" + str(self.explore_rate)
                self.log_handle.critical(log_str)
                # fake action
                instance_data.choose_action = 0
                instance_data.s_a_prob = 0
                instance_data.action_valid_info = [1 for i in range(self.model.a_output_count)]
                self.raw_instance_list.append(instance_data)
                self.check_sequnce(1)

                if len(self.choose_action_list) > 0:
                    self.send_message["actions"] = ','.join([str(i) for i in self.choose_action_list])
                if len(self.latency_data) > 0:
                    self.mean_latency_data.append(sum(self.latency_data)/len(self.latency_data))
                    self.send_message["clientLatency"] = json.dumps(self.mean_latency_data)
                if len(self.model_update_interval_list) > 0:
                    self.mean_model_update_interval_list.append(sum(self.model_update_interval_list)/len(self.model_update_interval_list))
                    self.send_message["actorModelUpdateInterval"] = json.dumps(self.mean_model_update_interval_list)
                    # json_str = json.dumps(self.model_update_interval_list)
                    # self.training_server_sender.send_string(
                    #     "actorModelUpdateInterval_%s_%s|||||%s" % (self.local_ip, self.worker_type, json_str))

            return -1


    def game_end_send_message(self):
        if len(self.Updatelong) > 0:
            self.send_message["Updatelong"] = ",".join([str(i) for i in self.Updatelong])
        if len(self.latencyLarge) > 0:
            self.send_message["latencyLarge"] = ",".join(self.latencyLarge)
        if len(self.timeoutlog) > 0:
            self.send_message["timeoutlog"] = self.timeoutlog[0]
        if len(self.modelUpdatelong) > 0:
            self.send_message["modelUpdatelong"] = ",".join([str(i) for i in self.modelUpdatelong])
        self.send_message["local_ip"] = self.local_ip
        self.send_message["opponent_name"] = self.opponent_name
        self.send_message["explore_rate"] = str(self.explore_rate)
        self.send_message["msg_type"] = "client_msg"
        self.send_message["finish_time"] = time.time()

        # self.send_message may incldue:
        #   local_ip, opponent_name, explore_rate, InstanceUpdateAll, actorModelUpdateInterval, clientLatency, actions,
        #   Updatelong, latencyLarge, timeoutlog, modelUpdatelong,
        #   log, clientlog
        p = pickle.dumps(self.send_message)
        self.training_server_sender.send(p)

