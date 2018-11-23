import tensorflow as tf
import numpy as np
import zmq
import json
import math
import time
import os
import copy
from Instance import Instance, Training_Instance
import zlib, pickle
from tensorflow.python.client import timeline



class ACNet:
    def readConfig(self):
        if self.scope.find("build") != -1:
            action_file = "./setting/build_action_config"
        elif self.scope.find("attack") != -1:
            action_file = "./setting/attack_action_config"
        with open(action_file, 'r') as f:
            all_action_str = f.read()
            all_action_str = all_action_str.strip()
            self.a_index_name_dict = {}
            for index, a in enumerate(all_action_str.split('&')):
                if a != "":
                    name, i = a.split(':')
                    self.a_index_name_dict[int(i)] = name
            self.a_output_count = len([i.split(':')[0] for i in all_action_str.split('&') if i != ""])
        feature_file = "./setting/feature_config"
        with open(feature_file, 'r') as f:
            all_feature_str = f.read()
            all_feature_str = all_feature_str.strip()
            self.feature_input_count = len([i.split(':')[0] for i in all_feature_str.split('&') if i != ""])
                                       #+ len(config.opponent_name_index_dict)
            self.action_related_feature_count = self.feature_input_count


    def __init__(self, scope):
        self.count = 0
        self.next_log_profile_time = 0

        self.scope = scope
        self.readConfig()
        self.serializd_model = "-1"
        self.duel_network = True
        self.use_layer_norm = True
        self.deserializing_assign_op = []
        self.deserializing_var_placeholder = {}
        self.lstm_max_length = 10

        with tf.variable_scope(scope):
            self.a_hit = tf.placeholder(tf.int32, None, name="input_choose_action")
            self.s = tf.placeholder(tf.float32, [None, self.feature_input_count], name="input_feature")
            self.batch_size = tf.placeholder(dtype=tf.int32, shape=[])

            self.a_Q_value = self.build_net(self.s)
            self.one_hot_result = tf.one_hot(self.a_hit, self.a_output_count, dtype=tf.float32)
            self.choose_action_Q_value = tf.reduce_sum(self.one_hot_result * self.a_Q_value, 1, keep_dims=True)
            self.clip_threshold = 0.1
            with tf.name_scope("compute_Q_target_choose_action"):
                self.action_zero_mask = tf.placeholder(tf.float32, [None, self.a_output_count],
                                                       name="input_valid_action_zero_mask")
                self.valid_action_q_value = self.a_Q_value * self.action_zero_mask

            with tf.name_scope('compute_Q_target_value'):
                self.terminal_reward = tf.placeholder(tf.float32, [None, 1], name="input_terminal_reward")
                # terminal state is 0 , none-terminal state is 1
                self.terminal_mask = tf.placeholder(tf.bool, [None, 1], name="input_terminal_mask")
                self.discount_rate = tf.placeholder(tf.float32, [None, 1])
                self.instant_reward = tf.placeholder(tf.float32, [None, 1])

                self.final_target_value = self.instant_reward + self.discount_rate * \
                tf.where(self.terminal_mask, self.terminal_reward, self.choose_action_Q_value)

            with tf.name_scope('compute_Q_gradient'):
                self.q_target = tf.placeholder(tf.float32, [None, 1], name="input_target_Q")
                self.weighted_IS = tf.placeholder(tf.float32, [None, 1])
                self.lr = tf.placeholder(tf.float32, None)

                self.td_error = self.q_target - self.choose_action_Q_value
                #weighted_priority = tf.pow(tf.abs(self.td_error), priority_exponent)
                self.a_td_error = tf.abs(tf.cast(tf.reshape(self.td_error, [-1]), tf.float32))

                self.a_params = tf.trainable_variables(self.scope + '/actor')
                self.a_l2_regulation_loss = tf.add_n([tf.nn.l2_loss(v) for v in self.a_params
                                                      if 'bias' not in v.name])

                huber_loss = tf.square(self.td_error) * 0.5
                self.a_object = tf.reduce_mean(self.weighted_IS * huber_loss)

                self.a_optimizer = tf.train.AdamOptimizer(self.lr)
                #self.a_optimizer = tf.train.RMSPropOptimizer(0.00025 / 4, decay=0.95, epsilon=1.5e-7)

                #(gradient, variable) list
                a_grads_origins = self.a_optimizer.compute_gradients(self.a_object, self.a_params)
                self.a_grads, self.a_global_grads = tf.clip_by_global_norm([g[0] for g in a_grads_origins], 10.0)
                self.a_server_apply_gradients = self.a_optimizer.apply_gradients(
                    [(g, a_grads_origins[index][1]) for index, g in enumerate(self.a_grads)])


    def serializing(self):
        a_vars = tf.trainable_variables(self.scope)
        a_values = tf.get_default_session().run(a_vars)
        a_var_values = {}
        for index, vars in enumerate(a_vars):
            a_var_values[vars.name] = a_values[index].tolist()

        p = pickle.dumps(a_var_values)
        z = zlib.compress(p)
        self.serializd_model = z #json.dumps(a_var_values)
        return z


    def deserializing(self, AC_str):
        p = zlib.decompress(AC_str)
        AC_total = pickle.loads(p)
        feed_dict = {}
        for var_name, value in AC_total.items():
            if var_name not in self.deserializing_var_placeholder:
                var = tf.get_default_session().graph.get_tensor_by_name(var_name)
                value = np.array(value)
                assign_placeholder = tf.placeholder(var.dtype, shape=value.shape)
                self.deserializing_var_placeholder[var_name] = assign_placeholder
                self.deserializing_assign_op.append(tf.assign(var, assign_placeholder))
            feed_dict[self.deserializing_var_placeholder[var_name]] = value
        tf.get_default_session().run(self.deserializing_assign_op, feed_dict=feed_dict)


    def huber_loss(self, x, delta):
        return tf.where(
            tf.abs(x) < delta,
            tf.square(x) * 0.5,
            delta * (tf.abs(x) - 0.5 * delta)
        )

    def layer_norm(self, input, shape, scope, layer_name):
        with tf.variable_scope(scope):
            mean, variance = tf.nn.moments(input, [1], keep_dims=True)
            normalised_input = (input - mean) / tf.sqrt(variance + 1e-10)
            #init to make zero mean and unit variance
            gains = tf.get_variable("norm_gain_" + layer_name, shape,
                                    initializer=tf.constant_initializer(1.))
            biases = tf.get_variable("norm_bias_" + layer_name, shape,
                                     initializer=tf.constant_initializer(0.))
            return normalised_input * gains + biases


    def build_net(self, feature_input):
        x_init = tf.contrib.layers.xavier_initializer()
        layer1_neuron_size = 200
        lstm_neuron_size = 150
        layer2_neuron_size = 100

        with tf.variable_scope("actor"):
            self.all_dense = tf.layers.dense(inputs=feature_input, units=layer1_neuron_size, activation=None,
                                                kernel_initializer=x_init,
                                                bias_initializer=tf.constant_initializer(0.))
            if self.use_layer_norm:
                self.all_dense_norm = self.layer_norm(self.all_dense, layer1_neuron_size, 'actor_all',
                                                         "all")
                all_dens_activation = tf.nn.relu(self.all_dense_norm)
            else:
                all_dens_activation = tf.nn.relu(self.all_dense)

            # lstm layer
            self.dropout_rate = tf.placeholder(tf.float32)
            self.lstm_input = tf.reshape(all_dens_activation,
                                         [self.batch_size, self.lstm_max_length, layer1_neuron_size])
            with tf.variable_scope("lstm", initializer=tf.orthogonal_initializer()):
                self.lstm_cell = tf.contrib.rnn.BasicLSTMCell(num_units=lstm_neuron_size,
                                                         state_is_tuple=True)
                self.lstm_cell_dropout = tf.nn.rnn_cell.DropoutWrapper(self.lstm_cell, input_keep_prob=self.dropout_rate, output_keep_prob=self.dropout_rate)
                # return state tupe (cell_state, hidden_state), each shape is (batch size, lstm_neuron_size)
                self.lstm_output, self.lstm_last_state = tf.nn.dynamic_rnn(
                    inputs=self.lstm_input, cell=self.lstm_cell_dropout, dtype=tf.float32)

            # only use the output of last block, shape(batch_size, lstm_neuron_size)
            self.bef_lstm_output, self.last_lstm_output = tf.split(self.lstm_output, [self.lstm_max_length - 1, 1], 1)
            self.batched_lstm_output = tf.reduce_mean(self.last_lstm_output, 1)
            self.value_dense1 = tf.layers.dense(inputs=self.batched_lstm_output, units=layer2_neuron_size, activation=None,
                                                kernel_initializer=x_init, bias_initializer=tf.constant_initializer(0.))
            if self.use_layer_norm:
                self.value_dense1_norm = self.layer_norm(self.value_dense1, layer2_neuron_size, 'actor1', "value_dense1")
                value_dense1_activation = tf.nn.relu(self.value_dense1_norm)
            else:
                value_dense1_activation = tf.nn.relu(self.value_dense1)
            self.state_vaule_output = tf.layers.dense(inputs=value_dense1_activation, units=1,
                                                       activation=None, kernel_initializer=x_init,
                                                       bias_initializer=tf.constant_initializer(0.))

            self.advantage_dense1 = tf.layers.dense(inputs=self.batched_lstm_output, units=layer2_neuron_size, activation=None,
                                                kernel_initializer=x_init,
                                                bias_initializer=tf.constant_initializer(0.))
            if self.use_layer_norm:
                self.advantage_dense1_norm = self.layer_norm(self.advantage_dense1, layer2_neuron_size, 'actor2', "advantage_dense1")
                advantage_dense1_activation = tf.nn.relu(self.advantage_dense1_norm)
            else:
                advantage_dense1_activation = tf.nn.relu(self.advantage_dense1)
            self.advantage_output = tf.layers.dense(inputs=advantage_dense1_activation, units=self.a_output_count,
                                                      activation=None, kernel_initializer=x_init,
                                                      bias_initializer=tf.constant_initializer(0.))

            self.actor_output_origin = self.state_vaule_output + (self.advantage_output -
                                   tf.reduce_mean(self.advantage_output, axis=1, keep_dims=True))
            return self.actor_output_origin


    def async_update(self):
        AC_str = self.receiver.recv()
        self.deserializing(AC_str)


    def training_init(self, training_server_sender, training_server_receiver):
        self.sender = training_server_sender
        self.receiver = training_server_receiver

    def training_socket_close(self):
        self.sender.close()
        self.receiver.close()

    def get_max_on_nonzero_items(self, valid_action_q_value):
        max_action_index = 0
        max_action_value = -99999
        for index, i in enumerate(valid_action_q_value):
            if i != 0 and i > max_action_value:
                max_action_value = i
                max_action_index = index
        return max_action_index

    def predict(self, instance, explore_rate, oppo_time, game_index):
        if sum(instance.action_valid_info) == 0:
            #self.logging.info("no valid action")
            return -1

        # predict_state_feature = np.concatenate([self.game[oppo_time][i] for i in instance.state_feature]).reshape(-1,self.feature_input_count )
        # predict_state_feature = np.array(self.game['self'][i] for i in instance.state_feature)
        predict_state_feature = np.concatenate([game_index[oppo_time].all_instances_features[i] for i in instance.state_feature]).reshape(-1, self.feature_input_count)

        valid_action_q_value, \
        lstm_output, lstm_last_state \
            = tf.get_default_session().run([self.valid_action_q_value, self.lstm_output, self.lstm_last_state],
                                           feed_dict={
                                               self.s: predict_state_feature,
                                               # self.s: np.array(instance.state_feature),
                                               self.action_zero_mask: np.array([instance.action_valid_info]),
                                               self.batch_size: 1,
                                               self.dropout_rate: 1.0
                                           })
        valid_action_q_value = valid_action_q_value.tolist()[0]
        max_action_index = self.get_max_on_nonzero_items(valid_action_q_value)
        sample_prob = np.random.randint(0, 100)
        total_valid_actions = sum(instance.action_valid_info)

        cur_valid_actions = [self.a_index_name_dict[index] + ":%.3f"%valid_action_q_value[index]
                             for index, i in enumerate(instance.action_valid_info) if i == 1]

        #do explore
        choose_action = max_action_index
        if sample_prob < explore_rate and total_valid_actions > 1:
            candidate_actions = []
            for index, p in enumerate(instance.action_valid_info):
                if index != max_action_index and p > 0:
                    candidate_actions.append(index)
            explore_index = np.random.randint(0, len(candidate_actions))
            choose_action = candidate_actions[explore_index]
        return choose_action


    def get_Q_value(self, instance, oppo_time, game_index):
        predict_state_feature = np.concatenate([game_index[oppo_time].all_instances_features[i] for i in
                                               instance.state_feature]).reshape(-1, self.feature_input_count)
        # predict_state_feature = np.concatenate([self.game[oppo_time][i] for i in instance.state_feature]).reshape(-1,
        #                             self.feature_input_count)
        valid_action_q_value \
            = tf.get_default_session().run(self.valid_action_q_value,
                                           feed_dict={
                                               self.s: predict_state_feature,
                                               # self.s: np.array(instance.state_feature),  # Worker instance
                                               self.action_zero_mask: np.array([instance.action_valid_info]),
                                               self.batch_size: 1,
                                               self.dropout_rate: 1.0
                                           })
        valid_action_q_value = valid_action_q_value.tolist()[0]
        max_action_index = self.get_max_on_nonzero_items(valid_action_q_value)
        return valid_action_q_value[max_action_index]


    def get_target_max_q_value_action(self, training_instances, is_server, game_index):
        s = self.get_lstm_input(training_instances, True, game_index)

        #options = tf.RunOptions(trace_level=tf.RunOptions.FULL_TRACE)
        #run_metadata = tf.RunMetadata()

        valid_action_q_value, all_q_values = \
            tf.get_default_session().run([self.valid_action_q_value, self.a_Q_value],
                                         feed_dict={
                                             self.s: s,
                                             self.action_zero_mask: np.concatenate(
                                                 [i.terminal_action_valid_info for i in training_instances], axis=0),
                                             self.batch_size: len(training_instances),
                                             self.dropout_rate: 1.0
                                         })#, options=options, run_metadata=run_metadata)

        #if is_server:
        #    if time.time() > self.next_log_profile_time:
        #        fetched_timeline = timeline.Timeline(run_metadata.step_stats)
        #        chrome_trace = fetched_timeline.generate_chrome_trace_format()
        #        with open('./profile_log/timeline_%s.json' % self.count, 'w') as f:
        #            f.write(chrome_trace)
        #        self.count += 1
        #        with open('./profile_log/batch_%d_%s' %(len(training_instances), self.count), 'w') as f:
        #            d = s.tolist()
        #            d_str = json.dumps(d)
        #            f.write(d_str)
        #        self.next_log_profile_time = time.time() + 30

        valid_action_q_value = valid_action_q_value.tolist()
        td_n_state_max_action = []
        for l in valid_action_q_value:
            td_n_state_max_action.append(self.get_max_on_nonzero_items(l))
        return td_n_state_max_action


    def get_target_q_value_estimation(self, training_instances, td_n_state_max_action, game_index):
        s = self.get_lstm_input(training_instances, True, game_index)

        target_value, choose_action_Q_value = \
            tf.get_default_session().run([self.final_target_value, self.choose_action_Q_value],
                                         feed_dict={
                                             self.s: s,
                                             self.a_hit: td_n_state_max_action,
                                             self.terminal_reward: np.array(
                                                 [[0] if i.is_end == "0" else [i.terminal_reward] for i in
                                                  training_instances]),
                                             self.terminal_mask: np.array(
                                                 [[False] if i.is_end == "0" else [True] for i in
                                                  training_instances]),
                                             self.discount_rate: [[i.td_discount] for i in training_instances],
                                             self.instant_reward: [[i.instant_rewards] for i in training_instances],
                                             self.batch_size: len(training_instances),
                                             self.dropout_rate: 1.0
                                         })
        return target_value


    def get_priority(self, training_instances, target_value, game_index):
        s = self.get_lstm_input(training_instances, False, game_index)
        instance_priority = \
            tf.get_default_session().run(self.a_td_error,
                                     feed_dict={
                                         self.s: s,
                                         self.a_hit: [i.choose_action for i in training_instances],
                                         self.q_target: target_value,
                                         self.batch_size: len(training_instances),
                                         self.dropout_rate: 1.0
                                     })
        return instance_priority.tolist()


    def sgd(self, training_instances, target_q_value, summary_logger,
            training_instances_priority, replay_buffer, model_type, game_index):
        instance_count = replay_buffer.get_total_receive_data_count()
        exponent_weight = -0.4

        lr = 0.00025 / 8

        weighted_IS = replay_buffer.cal_IS(training_instances_priority, exponent_weight, training_instances)
        s = self.get_lstm_input(training_instances, False, game_index)

        _, q_value_loss, q_value, \
        td_error_priority_before, grad_norm, \
        new_td_priority = \
            tf.get_default_session().run([self.a_server_apply_gradients,
                                          self.a_object, self.choose_action_Q_value,
                                          self.td_error, self.a_global_grads,
                                          self.a_td_error],

                                         feed_dict={
                                             self.s: s,
                                             self.a_hit: [i.choose_action for i in training_instances],
                                             self.q_target: target_q_value,
                                             self.weighted_IS: np.array(weighted_IS),
                                             self.batch_size: len(training_instances),
                                             self.lr: lr,
                                             self.dropout_rate: 0.8
                                         })
        td_error_priority = new_td_priority.tolist()

        summary_logger.add_summary('model/q_value_loss', q_value_loss)
        summary_logger.add_summary('model/avg_td_error', sum([abs(i[0]) for i in td_error_priority_before]) / len(td_error_priority_before))
        summary_logger.add_summary('model/avg_IS', sum([i[0] for i in weighted_IS]) / len(weighted_IS))
        summary_logger.add_summary('model/q_model_global_norm', grad_norm)
        summary_logger.add_summary('model/q_model_q_value', sum([i[0] for i in q_value]) / len(q_value))
        summary_logger.add_summary('model/target_q_value', sum(i[0] for i in target_q_value.tolist()) / len(target_q_value.tolist()))

        return td_error_priority


    def get_lstm_input(self, training_instances, is_convert_terminal, game_index):
        features = []
        if is_convert_terminal:
            for training_instance in training_instances:
                features.extend(game_index[training_instance.oppo_and_time].all_instances_features[i] for i in training_instance.terminal_state)
                # features.extend(self.game[training_instance.oppo_and_time][i] for i in training_instance.terminal_state)
        else:
            for training_instance in training_instances:
                features.extend(game_index[training_instance.oppo_and_time].all_instances_features[i] for i in training_instance.state_feature)
                # features.extend(self.game[training_instance.oppo_and_time][i] for i in training_instance.state_feature)
        s = np.concatenate(features).reshape((-1, self.feature_input_count))
        return s


