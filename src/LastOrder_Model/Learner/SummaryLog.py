import tensorflow as tf



class SummaryLog:
    def __init__(self, summary_writer):
        self.summary_writer = summary_writer
        self.tag_values_dict = {}
        self.tag_step_dict = {}
        self.tag_output_threshold_dict = {}
        self.tag_func_dict = {}
        self.tag_total_add_count = {}

    def add_tag(self, tag, output_threshold, cal_func):
        self.tag_values_dict[tag] = []
        self.tag_step_dict[tag] = 0
        self.tag_output_threshold_dict[tag] = output_threshold
        self.tag_func_dict[tag] = cal_func
        self.tag_total_add_count[tag] = 0

    def has_tag(self, tag):
        if tag in self.tag_step_dict.keys():
            return True
        else:
            return False

    def get_tag_count(self, tag):
        return self.tag_total_add_count[tag]

    def add_summary(self, tag, value):
        self.tag_values_dict[tag].append(value)
        self.tag_total_add_count[tag] += 1
        if len(self.tag_values_dict[tag]) >= self.tag_output_threshold_dict[tag]:
            summary = tf.Summary()
            avg_value = sum([self.tag_func_dict[tag](i) for i in self.tag_values_dict[tag]]) / len(self.tag_values_dict[tag])
            summary.value.add(tag=tag, simple_value=avg_value)
            self.summary_writer.add_summary(summary, self.tag_step_dict[tag])
            self.tag_step_dict[tag] += 1
            self.tag_values_dict[tag] = []

