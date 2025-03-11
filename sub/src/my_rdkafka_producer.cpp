#include "my_rdkafka_producer.h"

/* Producer Delivery_Report_Callback */
void Delivery_Report_Callback::dr_cb(RdKafka::Message &message){
    if(message.err()) std::cerr << "% Message delivery failed : " << message.errstr() << std::endl;
    else std::cerr << "% Message delivered to topic " << message.topic_name() 
    << "[" << message.partition() << "] at offset " 
    << message.offset() << std::endl;
}

// default constructor
Kafka_Producer::Kafka_Producer(){
    brokers = "127.0.0.1";
    topic = "sub0";
    freq = 0;
    Kafka_Producer::create_kafka_conf();
}

Kafka_Producer::Kafka_Producer(std::string brk, std::string tp, unsigned int f){
    brokers = brk;
    topic = tp;
    freq = f;

    std::cout << "PRODUCER : Set Broker : " << brokers << std::endl;
    std::cout << "PRODUCER : Set Topic : " << topic << std::endl;
    std::cout << "PRODUCER : Send Frequency : " << freq << "ms" << std::endl;

    Kafka_Producer::create_kafka_conf();
    Kafka_Producer::set_kafka_conf();
    Kafka_Producer::gen_kafka_producer();

}

void Kafka_Producer::create_kafka_conf(){
    conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
}

void Kafka_Producer::set_kafka_conf(){ //default
    // broker server set 
    if (conf->set("bootstrap.servers", brokers, err_str) != RdKafka::Conf::CONF_OK) {
        std::cerr << err_str << std::endl;
        exit(1);
    }

    // Delivery Report from prouucer Set
    if (conf->set("dr_cb", &dr_cb, err_str) != RdKafka::Conf::CONF_OK){
        std::cerr << err_str << std::endl;
        exit(1);
    }
}

void Kafka_Producer::set_kafka_conf(std::string set_string){
    // broker server set 
    if (conf->set(set_string, brokers, err_str) != RdKafka::Conf::CONF_OK) {
        std::cerr << err_str << std::endl;
        exit(1);
    }
}

void Kafka_Producer::gen_kafka_producer(){
    
    producer = RdKafka::Producer::create(conf, err_str);
    if(!producer){
        std::cerr << "Failed to create producer : " << err_str << std::endl;
        raise(SIGTERM);
    }
    delete conf;

}

/* request to sub_board Producer 생성. (1초에 한번씩 req message 를 m_req_topic 에 전달. */
/* main req_message example */
/*
{
    "msg_uuid" = "b9317db-02a2-4882-9b94-d1e1defe8c56",     // string
    "req_uuid" = "b912312312302a2-4882-9b94-dae123123",     // string
    "time_stamp" = 123123123123                             // int64
    "device" = "main_board",                                // string
    "msg_type" = "request",                                 // enum{"request", "retransmit", "stop"}
    "data" = {\"req\" : \"1\"},                             // data 는 json format의 string 으로 통일.
}
*/
void* Kafka_Producer::push_topic_t(void* arg){

    Thread_Args* args = static_cast<Thread_Args*>(arg);
    std::string generatedData = args->generator->generate();
    Kafka_Producer prd = *(args->producer);

    while(run){

        std::string msg = args->generator->generate();
        
        std::cout << " Producer Send : " << msg << std::endl;

        // 메세지 비어있는 경우,, 전송 안하고 Message Callback 
        if(msg.empty()){
            prd.producer->poll(0);
            continue;
        }

        //std::cout << "topic : " << prd.topic << std::endl;
        retry: 
        RdKafka::ErrorCode err = prd.producer->produce(
            prd.topic, /*topic name*/
            RdKafka::Topic::PARTITION_UA,   /* Any Partition */
            RdKafka::Producer::RK_MSG_COPY, /* Copy payload */
            const_cast<char*>(msg.c_str()), msg.size(), /* Message */
            NULL, 0, /*key*/
            0, /*time stamp (defaults to current time)*/
            NULL, 
            NULL
        );
        
        if(err != RdKafka::ERR_NO_ERROR){
            std::cerr << "% Failed to produce to topic " << prd.topic << ": " << RdKafka::err2str(err) << std::endl;

            if(err == RdKafka::ERR__QUEUE_FULL){ 
            /* Queue 가 full 났을 경우 Consumer 가 Message 가져갈때까지 기다림 */
                prd.producer->poll(1000); /* block for max 1000ms */
                goto retry; // 가져가면 재전송
            }
        }else{
            std::cerr << "% Enqueued Message (" << msg.size() << "bytes) " << "for topic " << prd.topic <<std::endl;
        }
        prd.producer->poll(0); //message delivered to topic - 메시지를 브로커로 전송한후, 성공적으로 전송되었는지 전달 보고서(Delivery Callback)를 콜백함. poll(0) 은 dr_cb를 호출 
            
        usleep(prd.freq);
    }

    std::cerr << "%  Flushing final Messages.. " << std::endl;
    prd.producer->flush(10 * 1000); /* wait for max 10 seconds */
    if(prd.producer-> outq_len() >0)
        std::cerr << "% " << prd.producer->outq_len() << "message(s) were not delivered" << std::endl;

    delete prd.producer; //너굴맨이 처리
    return nullptr;
    
}