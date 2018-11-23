import numpy
import time
import math
import copy
import json
from ACNet import ACNet
import tensorflow as tf
import numpy as np
from SummaryLog import SummaryLog
import pickle


class MinTree:
    def __init__(self, capacity):
        self.write = 0
        self.capacity = capacity
        self.tree = numpy.zeros(2 * capacity - 1)

    def _propagate(self, idx):
        parent = (idx - 1) // 2
        left = 2 * parent + 1
        right = left + 1

        if left < len(self.tree) and self.tree[left] != 0:
            left_value = self.tree[left]
            if right < len(self.tree) and self.tree[right] != 0:
                right_value = self.tree[right]
                self.tree[parent] = min(left_value, right_value)
            else:
                self.tree[parent] = left_value
        else:
            if right < len(self.tree) and self.tree[right] != 0:
                right_value = self.tree[right]
                self.tree[parent] = right_value
            else:
                raise Exception

        if parent != 0:
            self._propagate(parent)

    def add(self, p):
        idx = self.write + self.capacity - 1
        self.update(idx, p)
        self.write += 1
        if self.write >= self.capacity:
            self.write = 0

    def update(self, idx, p):
        self.tree[idx] = p
        self._propagate(idx)

    def get_min(self):
        return self.tree[0]


class SumTree:
    def __init__(self, capacity):
        self.write = 0
        self.capacity = capacity
        self.tree = numpy.zeros(2*capacity - 1)

    def _propagate(self, idx, change):
        parent = (idx - 1) // 2
        self.tree[parent] += change
        if parent != 0:
            self._propagate(parent, change)

    def _retrieve(self, idx, s):
        left = 2 * idx + 1
        right = left + 1
        if left >= len(self.tree):
            return idx

        #if right index is out of index, match this condition
        if s <= self.tree[left]:
            return self._retrieve(left, s)
        else:
            return self._retrieve(right, s-self.tree[left])

    def total_priority(self):
        return self.tree[0]

    def get_all_priority(self, end):
        return self.tree[self.capacity: end]

    def add(self, p):
        idx = self.write + self.capacity - 1
        self.update(idx, p)
        self.write += 1
        if self.write >= self.capacity:
            self.write = 0

    def update(self, idx, p):
        change = p - self.tree[idx]
        self.tree[idx] = p
        self._propagate(idx, change)

    def get(self, s):
        idx = self._retrieve(0, s)
        return (idx, self.tree[idx])



class ReplayBuffer:
    def __init__(self, single_capacity, action_index_name_dict, summary_logger, pool_name):
        config_file_path = "../StarcraftAITournamentManager-master/server/server_settings.json"
        all_opponent_bots_name = {}
        with open(config_file_path, 'r')as f:
            config_json = f.read()
            config_info = json.loads(config_json)
            for index, bot in enumerate(config_info["bots"]):
                if index == 0:
                    continue
                filter_bot_name = "".join(list(filter(str.isalpha, bot["BotName"])))
                if filter_bot_name not in all_opponent_bots_name:
                    all_opponent_bots_name[filter_bot_name] = 0
                all_opponent_bots_name[filter_bot_name] += 1
        self.excludeing_bots_name = set()
        tmp_list = []
        for name, count in all_opponent_bots_name.items():
            if count == 1:
                self.excludeing_bots_name.add(name)
            tmp_list.append(name)
        all_opponent_bots_name = tmp_list

        self.single_opponent_max_capacity = single_capacity
        self.opponent_name_to_pool_info = {}
        self.opponent_refesh_info = {}
        for index, opponent in enumerate(all_opponent_bots_name):
            self.opponent_name_to_pool_info[opponent] = { "start": index * self.single_opponent_max_capacity,
                                                          "end": (index + 1) * self.single_opponent_max_capacity,
                                                          "writer_position": index * self.single_opponent_max_capacity}
            self.opponent_refesh_info[opponent] = 0
            summary_logger.add_tag('%s_replay_buffer/%s_buffer_refresh_time' %(pool_name, opponent), 1, lambda x: x)

        capacity = self.single_opponent_max_capacity * len(all_opponent_bots_name)

        self.pool_name = pool_name
        self.data_count = 0
        self.capacity = capacity
        self.data = numpy.zeros(capacity, dtype=object)
        self.action_index_name_dict = action_index_name_dict
        self.next_print_time = 0
        self.priority_print_time = 0
        self.summary_logger = summary_logger
        self.start_add_data_time = 0
        self.total_receive_data_count = 0

        self.data_timeout_threshold = 60 * 60
        self.next_check_data_timeout_time = 0
        self.summary_logger = summary_logger
        summary_logger.add_tag('replay_buffer/erase_timeout_data', 1, lambda x: x)

        self.td_error_tree = SumTree(capacity)
        #self.action_priority_tree = SumTree(capacity)
        #self.min_tree = MinTree(capacity)
        self.priority_exponential_parameter = 0.6

        self.opponent_priority = {
            "Aiur": 2,
            "Ximp": 3,
            "Microwave": 3,
            "Arrakhammer": 2,
            "AIL": 2,
            "Steamhammer": 2,
            "LetaBot": 2,
            "Skynet": 2,
            "ZZZKBot": 2,

            "Juno": 2,
            "MegaBot": 2,
            "PurpleWave": 5,
            "McRave": 2,
            "Ziabot": 2,
            "ForceBot": 2
        }

        self.action_replic_setting = {
            "Attack_AllInAttack_natural": 2,
            "Attack_AllInAttack_other1": 3,
            "Attack_AllInAttack_other2": 3,
            "Attack_AllInAttack_other3": 3,
            "Attack_AllInAttack_start": 2,
            "Attack_MutaliskHarass_natural": 3,
            "Attack_MutaliskHarass_other1": 5,
            "Attack_MutaliskHarass_other2": 5,
            "Attack_MutaliskHarass_other3": 5,
            "Attack_MutaliskHarass_start": 3,
            "Attack_Airdrop_natural": 3,
            "Attack_Airdrop_other1": 5,
            "Attack_Airdrop_other2": 5,
            "Attack_Airdrop_other3": 5,
            "Attack_Airdrop_start": 3,
            "AllIn_addArmy": 1,
            "MutaliskHarass_addArmy": 1,
            "Building_Chamber": 1,
            "Building_Hive": 5,
            "Building_HydraliskDen": 1,
            "Building_Lair": 5,
            "Building_QueenNest": 1,
            "Building_SpawningPool": 1,
            "Building_Spire": 5,
            "Building_UltraliskCavern": 3,
            "Building_Hatchery": 1,
            "Building_Extractor": 1,
            "Defense_Spore_natural": 1,
            "Defense_Spore_start": 1,
            "Defense_Sunken_natural": 1,
            "Defense_Sunken_start": 1,
            "Expand_BaseExpand": 3,
            "Tech_LurkerTech": 5,
            "Unit_Hydralisk": 1,
            "Unit_Lurker": 3,
            "Unit_Mutalisk": 2,
            "Unit_Scourage": 1,
            "Unit_Ultralisk": 1,
            "Unit_Zergling": 1,
            "Unit_Drone": 1,
            "Upgrade_HydraliskRange": 1,
            "Upgrade_HydraliskSpeed": 1,
            "Upgrade_OverlordLoad": 1,
            "Upgrade_OverlordSpeed": 1,
            "Upgrade_OverlordSight": 1,
            "Upgrade_UltraliskArmor": 1,
            "Upgrade_UltraliskSpeed": 1,
            "Upgrade_ZerglingsAttackSpeed": 5,
            "Upgrade_ZerglingsSpeed": 3,
            "Upgrade_Zerg_Melee_Attacks": 1,
            "Upgrade_Zerg_Missile_Attacks": 1,
            "Upgrade_Zerg_Flyer_Attacks": 1,
            "Upgrade_Zerg_Carapace": 1,
            "Upgrade_Zerg_Flyer_Carapace": 1,
            "Wait_doNothing": 1
        }

    def save_replay_buffer(self, game_dict):
        used_game_id_set = set()
        valid_instances = []
        for item in self.data:
            if item == 0:
                continue
            used_game_id_set.add(item.oppo_and_time)
            #item.terminal_action_valid_info = item.terminal_action_valid_info.tolist()[0]
            valid_instances.append(item)
        valid_games = []
        for id, game in game_dict.items():
            if id in used_game_id_set:
                valid_games.append(game)
        with open('replay_buffer_instance.pickle', 'wb') as handle:
            pickle.dump(valid_instances, handle, protocol=pickle.HIGHEST_PROTOCOL)
        with open('replay_buffer_game.pickle', 'wb') as handle:
            pickle.dump(valid_games, handle, protocol=pickle.HIGHEST_PROTOCOL)


    def print_debug_info(self):
        if time.time() > self.next_print_time and self.data_count > 0:
            out_str = "%s " %self.pool_name
            out_str += "total instance:%d," %self.data_count
            out_str += "min priority:%f, " %self.get_min_priority()
            out_str += "total priority:%f " %self.total_priority()
            print(out_str)
            self.next_print_time = time.time() + 10
        #if time.time() > self.priority_print_time and self.data_count > 0:
        #    all_priority = self.td_error_tree.get_all_priority(self.data_count + self.capacity)
        #    cur_time = time.strftime('%H%M%S', time.localtime(time.time()))
        #    file_name = './plt_image/%s_%s'%(self.pool_name, cur_time)
        #    all_priority_sort = numpy.sort(all_priority)
        #    with open(file_name, 'w') as f:
        #        out_str = ""
        #        for item in all_priority_sort:
        #            out_str += str(item) + "\n"
        #        f.write(out_str)
        #    self.priority_print_time = time.time() + 60 * 10

        #if time.time() > self.next_check_data_timeout_time:
        #    erase_count = 0
        #    for index, item in enumerate(self.data):
        #        if item != 0:
        #            if time.time() - item.insert_pool_time > self.data_timeout_threshold:
        #                idx = index + self.capacity - 1
        #                self.td_error_tree.update(idx, 0)
        #                self.data[index] = 0
        #                erase_count += 1
        #    self.summary_logger.add_summary('replay_buffer/erase_timeout_data', erase_count)
        #    self.next_check_data_timeout_time = time.time() + 60 * 10

    def _cal_action_priority(self, action):
        action_name = self.action_index_name_dict[action]
        return self.action_replic_setting[action_name]

    def get_data_count(self):
        return self.data_count

    def get_min_priority(self):
        return 0.01 #self.min_tree.get_min()

    def total_priority(self):
        return self.td_error_tree.total_priority()

    def get_total_receive_data_count(self):
        return self.total_receive_data_count

    def add(self, p, data, opponent_name, exclude_bot):
        opponent_name = "".join(list(filter(str.isalpha, opponent_name)))
        if opponent_name in self.excludeing_bots_name and exclude_bot == True:
            return
        self.total_receive_data_count += 1
        self.data_count += 1
        self.data_count = self.capacity if self.data_count >= self.capacity else self.data_count

        if opponent_name not in self.opponent_name_to_pool_info:
            print("no opponent info!!!!!!!")
            return
        write_position = self.opponent_name_to_pool_info[opponent_name]["writer_position"]

        self.data[write_position] = data
        idx = write_position + self.capacity - 1
        self.update(idx, p)

        if write_position == self.opponent_name_to_pool_info[opponent_name]["start"]:
            self.opponent_refesh_info[opponent_name] = time.time()
        self.opponent_name_to_pool_info[opponent_name]["writer_position"] += 1
        if self.opponent_name_to_pool_info[opponent_name]["writer_position"] >= self.opponent_name_to_pool_info[opponent_name]["end"]:
            self.opponent_name_to_pool_info[opponent_name]["writer_position"] = self.opponent_name_to_pool_info[opponent_name]["start"]
            self.summary_logger.add_summary('%s_replay_buffer/%s_buffer_refresh_time'%(self.pool_name, opponent_name),
                                            time.time() - self.opponent_refesh_info[opponent_name])


    def update(self, idx, p):
        data_index = idx - self.capacity + 1
        action_replication_count = self._cal_action_priority(self.data[data_index].choose_action)

        time_min = int(self.data[data_index].time / (24 * 60))
        if time_min < 15:
            time_priority = 2.25
        elif time_min >= 15 and time_min < 30:
            time_priority = 1.5
        elif time_min >= 30:
            time_priority = 1

        opponent_name = self.data[data_index].opponent_name
        opponent_name = "".join(list(filter(str.isalpha, opponent_name)))
        opponent_priority = 1
        if opponent_name in self.opponent_priority:
            opponent_priority = self.opponent_priority[opponent_name]

        td_error_priority = p * action_replication_count * time_priority * opponent_priority
        weighted_priority = math.pow(td_error_priority, self.priority_exponential_parameter)
        self.data[data_index].priority = weighted_priority

        self.td_error_tree.update(idx, weighted_priority)
        #self.action_priority_tree.update(idx, action_priority)
        #self.min_tree.update(idx, td_error_priority)


    def get(self, s):
        td_tree_index, td_priority = self.td_error_tree.get(s)
        dataIdx = td_tree_index - self.capacity + 1
        return (td_tree_index, td_priority, self.data[dataIdx])


    def cal_IS(self, p_list, exponent_weight, instances):
        total_instance = self.data_count
        max_IS = math.pow(self.get_min_priority() * total_instance, exponent_weight)
        weighted_IS = [[math.pow(p * total_instance, exponent_weight) / max_IS] for p in p_list]
        return weighted_IS


s = '''


q_model = ACNet("build_q")
with tf.Session() as session:
    summary_writer = tf.summary.FileWriter("./summary/model_%s", session.graph)
    summary_logger = SummaryLog(summary_writer)
    r_pool = ReplayBuffer(5, q_model.a_index_name_dict, summary_logger, "build")
    for i in range(1, 15):
        r_pool.add(i, i * 2, "LetaBot")

    for i in range(1, 500):
        r_pool.add(i, i * 2, "LetaBot5")

    for i in range(1, 5):
        r_pool.add(i, i * 2, "LetaBot10")

    for i in range(1, 5):
        r_pool.add(i, i * 2, "LetaBot15")

    r_pool.print_debug_info()
    for i in range(10):
        sample_priority = np.random.randint(1, r_pool.total_priority())
        index, priority, data = r_pool.get(sample_priority)
        print("%d:%d:%d:%d"%(sample_priority, index, data, index - r_pool.capacity + 1))

'''


