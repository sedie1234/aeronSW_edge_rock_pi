/*
Consumer Example Code (Only Linux)
*/

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>

#include <sys/time.h>

#include <getopt.h>
#include <unistd.h>

#include <librdkafka/rdkafkacpp.h>

using namespace std; 

#define BROKER_SERVER "127.0.0.1"
#define BROKER_PORT "9092"
#define GROUP_ID "KETI_TEST"
#define TOPIC1 "test"
#define CODEC ""

static volatile sig_atomic_t run = 1;
static bool exit_eof             = false;
static int eof_cnt               = 0;
static int partition_cnt         = 0;
static int verbosity             = 1;
static long msg_cnt              = 0;
static int64_t msg_bytes         = 0;

static void print_time();
static void sigterm(int sig);
void msg_consume(RdKafka::Message *message, void *opaque);

class Event_Callback : public RdKafka::EventCb{
    public:
    void event_cb(RdKafka::Event &event){
        print_time();
        cout << " Kafka Event occur! " << event.type() << endl;
        switch (event.type()){
            case RdKafka::Event::EVENT_ERROR: // 카프카 에러 
                if (event.fatal()){
                    cerr << "Fatal ";
                    run = 0; 
                }
                cerr << "Error (" << RdKafka::err2str(event.err()) << "): " <<event.str() << endl; 
                break;
            
            case RdKafka::Event::EVENT_STATS: 
                cerr << "\"STATS\" " << event.str() << endl;
                break; 

            case RdKafka::Event::EVENT_LOG: 
                fprintf(stderr, "LOG-%i-%s: %s\n", event.severity(), event.fac().c_str(), event.str().c_str());
                break;

            case RdKafka::Event::EVENT_THROTTLE:
                cerr << "THROTTLED: " << event.throttle_time() << "ms by "
                            << event.broker_name() << " id " << (int)event.broker_id()
                            << endl;                    

            default:
                cerr << "@ EVENT "  << event.type() << "(" <<RdKafka::err2str(event.err()) << "): " << event.str() << endl;
                break;
        }
    }
};

class Rebalance_Callback : public RdKafka::RebalanceCb{
    private:
    static void part_list_print(const vector<RdKafka::TopicPartition*> &partitions){
        for (unsigned int i = 0; i < partitions.size(); i++){
            cerr << partitions[i]->topic() << "[" << partitions[i]->partition() << "]" << endl;
        }
    }

    public: 
    void rebalance_cb(RdKafka::KafkaConsumer* consumer, 
                    RdKafka::ErrorCode err, 
                    vector<RdKafka::TopicPartition*> &partitions){
        cerr << "Rebalance Callback : " << RdKafka::err2str(err) << ": ";
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
        //eof_cnt = 0; /* FIXME: Won't work with COOPERATIVE */

        if(error){
            cerr << "incremental assign failed: " << error->str() << endl;
            delete error;
        }else if(ret_err)
            cerr << "assign faield: " << RdKafka::err2str(ret_err) << endl;

    }
};



int main(){
    string brokers = BROKER_SERVER; 
    string topic1_str = TOPIC1;
    vector<string> topics;
    
    string err_str;
    string mode;
    string debug;

    bool do_conf_dump = false;

    // Create configuration objects
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    Rebalance_Callback rebalance_cb; 
    cout << "set" << endl;
    // @@@@@@@@@@@@@@@@@@@@@@@ Kafka Setting @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    //librdkafka CONFIGURATION.md 참고
    conf->set("rebalance_cb", &rebalance_cb ,err_str);
    conf->set("enable.partition.eof" ,"true", err_str);

    //Group ID 설정
    if(conf->set("group.id", (string)GROUP_ID, err_str) != RdKafka::Conf::CONF_OK){
        cerr << err_str << endl;
        exit(1);
    }

    cout << "1" << endl;

    //Interval 설정
    if(conf->set("statistics.interval.ms", "0", err_str) != RdKafka::Conf::CONF_OK){
        cerr << err_str << endl;
        exit(1);
    }

    //코덱 설정
    if(conf->set("compression.codec", "none", err_str) != RdKafka::Conf::CONF_OK){
        cerr << err_str << endl;
        exit(1);
    }

    /* Set Configuration Properties */
    conf->set("metadata.broker.list", brokers, err_str);

    //디버그 모드
    if(!debug.empty()){
        if(conf->set("debug", debug, err_str) != RdKafka::Conf::CONF_OK){
            cerr << err_str << endl;
            exit(1);
        }
    }
    // @@@@@@@@@@@@@@@@@@@@@@@ Kafka Setting end @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    /* Event Callback */
    cout << "event callback" << endl;
    Event_Callback event_cb;
    conf->set("event_cb", &event_cb, err_str);
    if (do_conf_dump) {
        list<string>* dump;
        dump = conf->dump();
        cout << "# Global Config " << endl;

        for(list<string>::iterator it = dump-> begin(); it != dump->end();  ){
            cout << *it << " = ";
            it++;
            cout << *it << endl;
            it++;
        }
        cout << endl;
        exit(0);
    }
    cout << "sss sss" << endl;
    signal(SIGINT, sigterm);
    signal(SIGTERM ,sigterm);

    //End 

    /* Consumer Mode */
    RdKafka::KafkaConsumer *consumer = RdKafka::KafkaConsumer::create(conf, err_str);

    //Create consumer using accumulated global configuration
    if(!consumer) {
        cerr << "Failed to create consumer: " << err_str << endl;
        exit(1);
    }

    delete conf;

    cout << " % Created Consumer " << consumer->name() << endl;

    /* Subscribe Topics */
    topics.push_back(TOPIC1);
    RdKafka::ErrorCode err = consumer->subscribe(topics);
    if(err){
        cerr << " Failed to subscribe to " << topics.size() << "topics: " << RdKafka::err2str(err) << endl; 
        exit(1);
    }

    /* Consume Messages */
    while(run){
        RdKafka::Message *msg = consumer->consume(1000); //Timeout_ms 1000
        msg_consume(msg, NULL);
        delete msg;
    }
    // 종료 코드
    alarm(10);
    consumer->close();
    delete consumer;

    cerr <<"% Consumed " << msg_cnt << "messages (" << msg_bytes << "bytes)" << endl;
    RdKafka::wait_destroyed(5000);

    return 0;
}


//종료 신호 
static void sigterm(int sig) {
    run = 0;
}

// format a string timestamp from the current time
static void print_time(){
    struct timeval tv;
    char buf[64];
    gettimeofday(&tv, NULL);
    strftime(buf, sizeof(buf)-1,  "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
    fprintf(stderr, "%s.%03d: ", buf, (int)(tv.tv_usec / 1000));
}

void msg_consume(RdKafka::Message* message, void* opaque){
    switch(message->err()){
        case RdKafka::ERR__TIMED_OUT:
            //cerr << "Time out" << endl;
            break;
        
        case RdKafka::ERR_NO_ERROR: /* Real Message */
            msg_cnt++;
            msg_bytes += message->len();
            
            //verbosity level 에 따라 표시하는 메세지 정보량 늘어남. default = 1
            if(verbosity >=3) cerr << "Read msg at offset " << message->offset() << endl;
            RdKafka::MessageTimestamp ts;
            ts = message->timestamp();

            if(verbosity >= 2 && ts.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE){
                string tsname = "?";
                if(ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME){
                    tsname = "create time";
                }else if(ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME){
                    tsname = "log append time";
                }
                cout << "Timestamp: " << tsname << " " << ts.timestamp << endl;
            }

            if(verbosity >= 2 && message->key()){
                cout << "Key : " << *message->key() << endl;
            }
            if(verbosity >= 1){
                cout << static_cast<int>(message->len()) << " " << static_cast<const char*>(message->payload());
            }

        case RdKafka::ERR__PARTITION_EOF: /* Last Message */
            if(exit_eof && ++eof_cnt == partition_cnt){
                cerr << "%% EOF Reached for all " << partition_cnt << "partition(s)" << endl;
                run = 0;
            }
            break;
        
        default: /* err */
            cerr << " Consume Failed : " << message->errstr() << endl;
            run = 0;
    }
}