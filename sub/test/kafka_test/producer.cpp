#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>

#include <librdkafka/rdkafkacpp.h>
using namespace std;

static volatile sig_atomic_t run = 1;
static void sigterm(int sig);

class Delivery_Report_Callback : public RdKafka::DeliveryReportCb{

    public:
    // if message.err() is non-zero that the message delivery failed permanetly
    void dr_cb(RdKafka::Message &message){
        if(message.err()) cerr << "% Message delivery failed : " << message.errstr() << endl;
        else cerr << "% Message delivered to topic " << message.topic_name() 
        << "[" << message.partition() << "] at offset " 
        << message.offset() << endl;
    }
};

int main(){

    string brokers = "127.0.0.1";
    string topic = "test";

    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    string err_str;

    /* Set bootstrap broker(s) as a comma-separated list of
    * host or host:port (default port 9092).
    * librdkafka will use the bootstrap brokers to acquire the full
    * set of brokers from the cluster. */
    if (conf->set("bootstrap.servers", brokers, err_str) != RdKafka::Conf::CONF_OK) {
        std::cerr << err_str << std::endl;
        exit(1);
    }
    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);

    Delivery_Report_Callback dr_cb;
    if (conf->set("dr_cb", &dr_cb, err_str) != RdKafka::Conf::CONF_OK){
        cerr << err_str << endl;
        exit(1);
    }

    /* Create producer instance */
    RdKafka::Producer *producer = RdKafka::Producer::create(conf, err_str);
    if(!producer){
        cerr << "Failed to create producer : " << err_str << endl;
        exit(1);
    }
    delete conf;
    
    // Read Message from stdin and produce to broker.
    cout << "" << endl;


    //문자열 입력
    for(string line; run && getline(cin, line); ){
        if(line.empty()){
            producer->poll(0);
            continue;
        }
    
        /*
        * Send/Produce message.
        * This is an asynchronous call, on success it will only
        * enqueue the message on the internal producer queue.
        * The actual delivery attempts to the broker are handled
        * by background threads.
        * The previously registered delivery report callback
        * is used to signal back to the application when the message
        * has been delivered (or failed permanently after retries).
        */
        
        retry: 
        RdKafka::ErrorCode err = producer->produce(
            topic, /*topic name*/
            RdKafka::Topic::PARTITION_UA, /* Any Partition */
            RdKafka::Producer::RK_MSG_COPY, /* Copy payload */
            const_cast<char *>(line.c_str()), line.size(), 
            NULL, 0, /*key*/
            0, /*time stamp (defaults to current time)*/
            NULL, 
            NULL
        );

        if(err != RdKafka::ERR_NO_ERROR){
            cerr << "% Failed to produce to topic " << topic << ": " << RdKafka::err2str(err) << endl;

            if(err == RdKafka::ERR__QUEUE_FULL){ 
            /* Queue 가 full 났을 경우 Consumer 가 Message 가져갈때까지 기다림 */
                producer->poll(1000); /* block for max 1000ms */
                goto retry;
            }
        }else{
            cerr << "% Enqueued Message (" << line.size() << "bytes) " << "for topic " << topic <<endl;
        }
        producer->poll(0);
    }

    cerr << "%  Flushing final Messages.. " << endl;
    producer->flush(10 * 1000); /* wait for max 10 seconds */
    if(producer-> outq_len() >0)
        cerr << "% " << producer->outq_len() << "message(s) were not delivered" << endl;

    delete producer;

    return 0;

}

static void sigterm(int sig){
    run = 0;
}

