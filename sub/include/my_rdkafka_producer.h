#ifndef _MY_RDKAFKA_PRODUCER_H
#define _MY_RDKAFKA_PRODUCER_H
#include <iostream>
#include <unistd.h>
#include <chrono>
#include "kafka/rdkafkacpp.h"
#include "sig_handler.h"

//데이터 생성 관련 Header
#include "data_generator.h"

/* Producer */
class Delivery_Report_Callback : public RdKafka::DeliveryReportCb{
    public:
    // if message.err() is non-zero that the message delivery failed permanetly
    void dr_cb(RdKafka::Message &message);
};

class Kafka_Producer{
    private:
    RdKafka::Conf* conf;
    std::string err_str;
    Delivery_Report_Callback dr_cb;
    
    public:
    std::string brokers;
    std::string topic;
    unsigned int freq;
    
    RdKafka::Producer* producer;

    Kafka_Producer();
    Kafka_Producer(std::string broker, std::string topic, unsigned int freq); 
    void create_kafka_conf();                               //conf 객체 생성 
    void set_kafka_conf();                                  //default 설정.
    void set_kafka_conf(std::string set_string);            //추가로 더 설정해야하는거 있을떄 사용. 지금은 X
    void gen_kafka_producer();                              //conf 를 사용해 producer 생성후 conf 삭제.
    static void* push_topic_t(void* data);                  //push 쓰레드 -> static
    
};

//Thread args
struct Thread_Args{
    std::shared_ptr<IData_Generator> generator;     // Input Data generate Class Args
    Kafka_Producer* producer;                       // Kafka Producer Class Args
};

#endif