#include "my_rdkafka_consumer.h"

//여기서만 사용
static bool exit_eof             = false;
static int eof_cnt               = 0;
static int partition_cnt         = 0;
static int verbosity             = 1;
static long msg_cnt              = 0;
static int64_t msg_bytes         = 0;

static void print_time();

/* Consumer Event Callback */
void Event_Callback::event_cb(RdKafka::Event &event){
    print_time();
    std::cout << " Kafka Event occur! " << event.type() << std::endl;
    switch (event.type()){
        case RdKafka::Event::EVENT_ERROR: // 카프카 에러 
            if (event.fatal()){
                std::cerr << "Fatal ";
                run = 0; 
            }
            std::cerr << "Error (" << RdKafka::err2str(event.err()) << "): " <<event.str() << std::endl; 
            break;
        
        case RdKafka::Event::EVENT_STATS: 
            std::cerr << "\"STATS\" " << event.str() << std::endl;
            break; 

        case RdKafka::Event::EVENT_LOG: 
            fprintf(stderr, "LOG-%i-%s: %s\n", event.severity(), event.fac().c_str(), event.str().c_str());
            break;

        case RdKafka::Event::EVENT_THROTTLE:
            std::cerr << "THROTTLED: " << event.throttle_time() << "ms by "
                        << event.broker_name() << " id " << (int)event.broker_id()
                        << std::endl;      
            break;              

        default:
            std::cerr << "@ EVENT "  << event.type() << "(" <<RdKafka::err2str(event.err()) << "): " << event.str() << std::endl;
            break;
    }
}

/* Consumer Rebalance_Callback */
void Rebalance_Callback::part_list_print(const std::vector<RdKafka::TopicPartition*> &partitions){
    for (unsigned int i = 0; i < partitions.size(); i++){
        std::cerr << partitions[i]->topic() << "[" << partitions[i]->partition() << "]" << std::endl;
    }
}

void Rebalance_Callback::rebalance_cb(RdKafka::KafkaConsumer* consumer, RdKafka::ErrorCode err, 
                std::vector<RdKafka::TopicPartition*> &partitions){
    std::cerr << "Rebalance Callback : " << RdKafka::err2str(err) << ": ";
    part_list_print(partitions);
    RdKafka::Error* error = NULL;
    RdKafka::ErrorCode ret_err = RdKafka::ERR_NO_ERROR;

    if(err == RdKafka::ERR__ASSIGN_PARTITIONS){
        if(consumer->rebalance_protocol() == "COOPERATIVE") error = consumer->incremental_assign(partitions); 
        else ret_err = consumer->assign(partitions);

        partition_cnt += (int)partitions.size();

    }else{
        if(consumer->rebalance_protocol() == "COOPERATIVE"){
            error = consumer->incremental_unassign(partitions);
            partition_cnt -= (int)partitions.size();
        }else{
            ret_err = consumer->unassign();
            partition_cnt = 0; 
        }
    }
    eof_cnt = 0; /* FIXME: Won't work with COOPERATIVE */

    if(error){
        std::cerr << "incremental assign failed: " << error->str() << std::endl;
        delete error;
    }else if(ret_err)
        std::cerr << "assign faield: " << RdKafka::err2str(ret_err) << std::endl;

}

Kafka_Consumer::Kafka_Consumer(){
    topics = {"main_req"};
    brokers = "127.0.0.1";
    group_id = "sub0";
    Kafka_Consumer::create_kafka_conf();
}

Kafka_Consumer::Kafka_Consumer(std::string brk, std::vector<std::string> tps, std::string g_id){
    topics = tps;
    brokers = brk;
    group_id = g_id;
    Kafka_Consumer::create_kafka_conf();
}

void Kafka_Consumer::create_kafka_conf(){
    conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
}

void Kafka_Consumer::set_kafka_conf(){
    //librdkafka CONFIGURATION.md 참고
    conf->set("rebalance_cb", &rebalance_cb ,err_str);
    conf->set("enable.partition.eof" ,"true", err_str);
    //Group ID 설정
    if(conf->set("group.id", (std::string)group_id, err_str) != RdKafka::Conf::CONF_OK){
        std::cerr << err_str << std::endl;
        exit(1);
    }
    //Interval 설정
    if(conf->set("statistics.interval.ms", "0", err_str) != RdKafka::Conf::CONF_OK){
        std::cerr << err_str << std::endl;
        exit(1);
    }
    //코덱 설정
    // if(conf->set("compression.codec", "none", err_str) != RdKafka::Conf::CONF_OK){
    //     cerr << err_str << endl;
    //     exit(1);
    // }
    /* Set Configuration Properties */
    conf->set("metadata.broker.list", brokers, err_str);

    //디버그 모드
    if(!debug.empty()){
        if(conf->set("debug", debug, err_str) != RdKafka::Conf::CONF_OK){
            std::cerr << err_str << std::endl;
            exit(1);
        }
    }

    // Event Callback
    conf->set("event_cb", &event_cb, err_str);

    // conf dump 후 출력 
    bool do_conf_dump = false;
    if (do_conf_dump) {
        std::list<std::string>* dump;
        dump = conf->dump();
        std::cout << "# Global Config " << std::endl;

        for(std::list<std::string>::iterator it = dump-> begin(); it != dump->end();  ){
            std::cout << *it << " = ";
            it++;
            std::cout << *it << std::endl;
            it++;
        }
        std::cout << std::endl;
        //exit(0);
    }
}

void Kafka_Consumer::gen_kafka_consumer(){
    consumer = RdKafka::KafkaConsumer::create(conf, err_str);
    if(!consumer){
        std::cerr << "Failed to create producer : " << err_str << std::endl;
        exit(1);
    }else{
        std::cout << " % Created Consumer " << consumer->name() << std::endl;
    }
    //setting 한 conf 로 consumer 만들었으니, 이제 이 객체에서는 더이상 안씀. 따라서 conf 에 할당한 메모리 삭제.
    delete conf; 
}

void* Kafka_Consumer::pull_topic_t(void* data){
    Rapid_Json_Handler json_handler;
    int msg_len;
    const char* msg_payload;
    Kafka_Consumer cons = *static_cast<Kafka_Consumer*>(data);
    RdKafka::ErrorCode err = cons.consumer->subscribe(cons.topics);

    if(err){
        std::cerr << " Failed to subscribe to " << cons.topics.size() << "topics: " << RdKafka::err2str(err) << std::endl; 
        exit(1);
    }
    
    while(run){

        /* req message 전송 받았을때 처리 */
        RdKafka::Message* msg = cons.consumer->consume(1); //Timeout_ms 1000
        RdKafka::MessageTimestamp ts;
        ts = msg->timestamp();

        /* Time out*/
        if(msg->err() == RdKafka::ERR__TIMED_OUT){
            //std::cerr << "Time out" << std::endl;
        }

        /* Real Message */    
        else if(msg->err() == RdKafka::ERR_NO_ERROR){
            msg_cnt++;
            msg_bytes += msg->len();
            msg_len = static_cast<int>(msg->len());
            msg_payload = static_cast<const char*>(msg->payload());
            //std::cout << msg_len << " " << msg_payload << std::endl;
            
            //Kafka Timestamp

            RdKafka::MessageTimestamp ts;
            ts = msg->timestamp();

            if(ts.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE){
                std::string tsname = "?";
                if(ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME){
                    tsname = "create time"; //Producer 가 메세지를 생성한 시간
                }else if(ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME){
                    tsname = "log append time"; //메세지가 Kafka 브로커 서버에 들어온 시간
                }
                std::cout << "Timestamp: " << tsname << " " << ts.timestamp << std::endl;
            }
            if(msg->key()){
                std::cout << "Key : " << *msg->key() << std::endl;
            }
            
            // 메세지 파싱
            //string 타입
            auto json_msg_uuid = std::get<std::string>(json_handler.json_parsing(msg_payload, "/payload/msg_uuid"));
            std::cout << "msg_uuid : " << json_msg_uuid << std::endl;
            req_id = json_msg_uuid;
            auto json_msg_type = std::get<std::string>(json_handler.json_parsing(msg_payload, "/payload/msg_type"));
            std::cout << "msg_type : " << json_msg_type << std::endl;
            auto json_data = std::get<std::string>(json_handler.json_parsing(msg_payload, "/payload/data"));
            std::cout << "msg_json/data : " << json_data << std::endl;
            auto json_req = std::get<std::string>(json_handler.json_parsing(json_data.c_str(), "/req"));                       
            std::cout << "msg_json/data/req : " << json_req << std::endl;

            // auto json_vec = std::get<std::vector<std::string>>(json_handler.json_parsing(msg_payload, "/data_vector_type_test"));
            // for(std::string v : json_vec){
            //     std::cout << v << " " << std::endl;
            // }

            if(json_req == "r"){
                // producer 동작 인터럽트(req_sig) 발생. raise(req_sig); 프로듀서 측에서는 req_sig
                raise(SIGUSR1);
            }
            
        } 

        /* Last Message */
        else if(msg->err() == RdKafka::ERR__PARTITION_EOF){
            if(exit_eof && ++eof_cnt == partition_cnt){
                std::cerr << "%% EOF Reached for all " << partition_cnt << "partition(s)" << std::endl;
                run = 0;
            }
        } 
        /* err */
        else{
            std::cerr << " Consume Failed : " << msg->errstr() << std::endl;
            run = 0;
        }
        delete msg;
    }

    alarm(10);
    cons.consumer->close();
    delete cons.consumer;
    std::cerr <<"% Consumed " << msg_cnt << "messages (" << msg_bytes << "bytes)" << std::endl;
    RdKafka::wait_destroyed(1000);
    return nullptr;
}

// format a string timestamp from the current time
static void print_time(){
    struct timeval tv;
    char buf[64];
    gettimeofday(&tv, NULL);
    strftime(buf, sizeof(buf)-1,  "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
    fprintf(stderr, "%s.%03d: ", buf, (int)(tv.tv_usec / 1000));
}

// Consumer -- Msg Consume
void msg_consume(RdKafka::Message* message, void* opaque){
    (void)opaque;
    switch(message->err()){
        case RdKafka::ERR__TIMED_OUT:
            //cerr << "Time out" << endl;
            break;
        
        case RdKafka::ERR_NO_ERROR: /* Real Message */
            msg_cnt++;
            msg_bytes += message->len();
            
            //verbosity level 에 따라 표시하는 메세지 정보량 늘어남. default = 1
            if(verbosity >=3) std::cerr << "Read msg at offset " << message->offset() << std::endl;

            RdKafka::MessageTimestamp ts;
            ts = message->timestamp();

            if(verbosity >= 2 && ts.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE){
                std::string tsname = "?";
                if(ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME){
                    tsname = "create time";
                }else if(ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME){
                    tsname = "log append time";
                }
                std::cout << "Timestamp: " << tsname << " " << ts.timestamp << std::endl;
            }
            if(verbosity >= 2 && message->key()){
                std::cout << "Key : " << *message->key() << std::endl;
            }

            if(verbosity >= 1){
                std::cout << static_cast<int>(message->len()) << " " << static_cast<const char*>(message->payload());
            }
            break;

        case RdKafka::ERR__PARTITION_EOF: /* Last Message */
            if(exit_eof && ++eof_cnt == partition_cnt){
                std::cerr << "%% EOF Reached for all " << partition_cnt << "partition(s)" << std::endl;
                run = 0;
            }
            break;
        
        default: /* err */
            std::cerr << " Consume Failed : " << message->errstr() << std::endl;
            run = 0;
            break;
    }
}
