import numpy as np
import time

class Instance:
    def __init__(self, time, state_feature, choose_action, s_a_prob, reward, terminal_reward, is_end, action_valid_info,
                 attack_info):
        self.time = time
        #float
        self.state_feature = state_feature
        #int
        self.choose_action = choose_action
        #float
        self.s_a_prob = s_a_prob
        #float
        self.reward = reward

        #float
        self.terminal_reward = terminal_reward
        #string
        self.is_end = is_end
        self.action_valid_info = action_valid_info

        #float
        self.attack_info = attack_info


class Training_Instance:
    def __init__(self, time_stamp, state_feature, choose_action, td_discount, terminal_state, terminal_action_valid_info,
                 terminal_reward, is_end, instant_rewards, priority, Q_reward, explore_rate, opponent_name, oppo_and_time):

        self.time = time_stamp
        # float
        self.state_feature = state_feature
        # int
        self.choose_action = choose_action

        #float
        self.td_discount = td_discount
        #float
        self.terminal_state = terminal_state
        #string list
        self.terminal_action_valid_info = terminal_action_valid_info
        #float
        self.terminal_reward = terminal_reward
        #string
        self.is_end = is_end
        #float
        self.instant_rewards = instant_rewards

        #float
        self.priority = priority
        #float
        self.Q_reward = Q_reward

        self.explore_rate = explore_rate

        self.opponent_name = opponent_name

        self.insert_pool_time = time.time()

        self.oppo_and_time = oppo_and_time


    def trans_nparray(self):
        # self.terminal_state = np.array([self.terminal_state])
        # self.state_feature = np.array([self.state_feature])
        self.terminal_action_valid_info = np.array([self.terminal_action_valid_info])



def as_payload(dct):
    return Training_Instance(dct["time"], dct["state_feature"], dct["choose_action"],
                             dct["td_discount"], dct["terminal_state"],
                             dct["terminal_action_valid_info"],
                             dct["terminal_reward"], dct["is_end"], dct["instant_rewards"],
                             dct["priority"], dct["Q_reward"], dct["explore_rate"],
                             dct["opponent_name"], dct["td_n_instance_time"])


class Game:
    def __init__(self, all_instances_features, training_instances, oppo_and_time):
        # numpy.array
        self.all_instances_features = all_instances_features
        # Training_instance
        self.training_instances = training_instances
        # string
        self.oppo_and_time = oppo_and_time

        self.instance_timestamp = []



