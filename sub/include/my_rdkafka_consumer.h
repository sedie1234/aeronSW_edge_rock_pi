#ifndef _MY_RDKAFKA_CONSUMER_H
#define _MY_RDKAFKA_CONSUMER_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>

#include <cstring>
#include <sys/time.h>
#include <getopt.h>
#include <unistd.h>

//#include <librdkafka/rdkafkacpp.h>
#include "kafka/rdkafkacpp.h"
#include "sig_handler.h"
#include "utils/utils.h"

//JSON 메시지 파싱 위한 헤더
#include "rapid_json_handler.h"


/* Consumer */
class Event_Callback : public RdKafka::EventCb{
    public:
    void event_cb(RdKafka::Event &event);
};

class Rebalance_Callback : public RdKafka::RebalanceCb{
    private:
    static void part_list_print(const std::vector<RdKafka::TopicPartition*> &partitions);

    public: 
    void rebalance_cb(RdKafka::KafkaConsumer* consumer, 
                    RdKafka::ErrorCode err, 
                    std::vector<RdKafka::TopicPartition*> &partitions);
};

class Kafka_Consumer{
    private:
    RdKafka::Conf* conf;
    Rebalance_Callback rebalance_cb;
    Event_Callback event_cb;

    std::string err_str;
    std::string debug;

    public:
    std::vector<std::string> topics;
    std::string brokers;
    std::string group_id;

    RdKafka::KafkaConsumer* consumer;

    Kafka_Consumer();
    Kafka_Consumer(std::string brk, std::vector<std::string> tps, std::string g_id);
    void create_kafka_conf();
    void set_kafka_conf();
    void gen_kafka_consumer();
    static void* pull_topic_t(void* data);

};

void msg_consume(RdKafka::Message* message, void* opaque);

#endif