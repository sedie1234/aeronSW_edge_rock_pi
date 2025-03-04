// only linux
// Jetson Orin CPU 코어 개수 : 12-core + 12 threads

// 서브보드 코드

#include <thread>
#include "my_rdkafka_producer.h"
#include "my_rdkafka_consumer.h"
#include "sig_handler.h"
#include "utils/utils.h"

const int MSG_SIZE = 1024;
const unsigned int FREQ = 1000000; //us (1,000,000 = 1sec)

//Producer Define --> 나중에 config 파일 읽어와서 넣는걸로 변경
// const std::string PRD_BROKER = "10.252.218.115";
// const std::string PRD_TOPIC = "sub_board_cam";

// //Consumer Define
// const std::string CONS_BROKER = "10.252.218.115";
// const std::vector<std::string> CONS_TOPICS = {"test"}; 
// const std::string CONS_GROUP_ID = "sub1";

//Producer Define --> 나중에 config 파일 읽어와서 넣는걸로 변경
const std::string PRD_BROKER = "192.168.0.205";
const std::string PRD_TOPIC = "sub0";

//Consumer Define
const std::string CONS_BROKER = "192.168.0.205";
const std::vector<std::string> CONS_TOPICS = {"main_req"}; 
const std::string CONS_GROUP_ID = "sub0";

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<std::string> integ_vector;

std::vector<pthread_t*> producer_thread_Manager; 
std::vector<pthread_t*> consumer_thread_Manager; 

Kafka_Producer set_prd_status(const std::string broker, std::string topic, unsigned int FREQ);
Kafka_Consumer set_cons_status(const std::string broker, std::vector<std::string> topic, std::string group_id);
void* thread_handler(void* data);

int main(){

    setup_sig_handler();
    auto th_handler = std::unique_ptr<pthread_t, PThreadDeleter>(new pthread_t);
    std::cout << "th_handler addr : " << &th_handler << std::endl;

    if(pthread_create(th_handler.get(), nullptr, thread_handler, nullptr) != 0){
        std::cerr << "failed generate Thread " << std::endl; 
        return -1;    
    }

    for(std::string line; run && getline(std::cin, line); ){
        
        if(line == "li" or line=="list"){
            std::cout << "Connected Producer List..." << std::endl;
            //TODO

        }else if(line == "restart"){
            std::cout << "restart server" << std::endl;       
            //TODO 

        }else if(line == "exit"){
            run = 0; 
            break;
        }else{   
        }
    }
    
    pthread_mutex_destroy(&mutex);

    return 0;
}

void* thread_handler(void* data){
    (void)data;

    /* ============================== 
     * ======= Consumer Part ======== 
     * ============================== */
    Kafka_Consumer cons = set_cons_status(CONS_BROKER, CONS_TOPICS, CONS_GROUP_ID); 
    pthread_t pull_data_t;
    if(pthread_create(&pull_data_t, nullptr, Kafka_Consumer::pull_topic_t, &cons) != 0){
    std::cerr << "failed generate consumer Thread " << std::endl; 
        perror("consumer_threads");
        exit(1);
    }else{
        consumer_thread_Manager.push_back(&pull_data_t);
    }

    /* ============================== 
     * ======= Producer Part ======== 
     * ============================== */
    Kafka_Producer prd = set_prd_status(PRD_BROKER, PRD_TOPIC, 1000); //broker, topic, frequency(u_sec), ... 추가
    pthread_t push_data_t;
    if(pthread_create(&push_data_t, nullptr, Kafka_Producer::push_topic_t, &prd) != 0){
        std::cerr << "failed generate producer Thread " << std::endl; 
        perror("producer_threads");
        exit(1);
    }else{
        producer_thread_Manager.push_back(&push_data_t);
    }

    while(run){ sleep(1); };

    //종료 코드 - Consumer, Producer Thread 종료까지 대기 
    for (pthread_t* prd_thread : producer_thread_Manager) {
        pthread_join(*prd_thread, nullptr);  
        std::cout << "Deallocate producer_thread : " << *prd_thread << std::endl;
    }
    // producer_thread_Manager.clear();
    for (pthread_t* cons_thread : consumer_thread_Manager) {
        pthread_join(*cons_thread, nullptr); 
        std::cout << "deallocate consumer_thread : " << *cons_thread << std::endl;
    }
    // consumer_thread_Manager.clear();
    return nullptr;

}

Kafka_Producer set_prd_status(const std::string broker, std::string topic, unsigned int freq){
    Kafka_Producer prd(broker, topic, freq);
    prd.set_kafka_conf();
    prd.gen_kafka_producer();
    return prd;
}

Kafka_Consumer set_cons_status(const std::string broker, std::vector<std::string> topics, std::string group_id){
    Kafka_Consumer cons(broker, topics, group_id); 
    cons.set_kafka_conf();
    cons.gen_kafka_consumer(); 
    return cons;
} 