// only linux
// Jetson Orin CPU 코어 개수 : 12-core + 12 threads

// 메인보드 코드

#include <thread>

#include "my_rdkafka_producer_lib.h"
#include "my_rdkafka_consumer_lib.h"
#include "sig_handler.h"
#include "utils.h"

const int MSG_SIZE = 1024;

//Producer Define
const std::string PRD_BROKER = "127.0.0.1";
const std::string PRD_TOPIC = "req_test";

//Consumer Define
const std::string CONS_BROKER = "127.0.0.1";
const std::string CONS_GROUP_ID = "main";
const std::vector<std::string> CONS_TOPICS = {"sub1_topic_test","sub2_topic_test"}; 

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

std::vector<pthread_t*> producer_thread_Manager; 
std::vector<pthread_t*> consumer_thread_Manager; 

void* thread_handler(void* data);

int main(){
    setup_sig_handler();

    pthread_t th_handler;
    if(pthread_create(&th_handler, nullptr, thread_handler, nullptr) != 0){
        std::cerr << "failed generate Thread " << std::endl; 
        return -1;    
    }

    for(std::string line; run && getline(std::cin, line); ){
        
        if(line == "li" or line=="list"){
            std::cout << "deving..." << std::endl; // --> TODO 

        }else if(line == "restart"){
            std::cout << "restart server" << std::endl;       
            //raise(SIGUSR1); --> TODO 

        }else if(line == "exit"){
            raise(SIGTERM);
            break;
        }else{   
        }
    }

    //종료 코드
    pthread_detach(th_handler); // 리소스 자동 해제 
    std::cout << "deallocate thread handler : " << th_handler << std::endl;
    pthread_mutex_destroy(&mutex);

    return 0;
}

void* thread_handler(void* data){
    (void)data;

    // ====== Producer Part ======
    Kafka_Producer prd(PRD_BROKER, PRD_TOPIC);
    prd.set_kafka_conf();
    prd.gen_kafka_producer();
    
    pthread_t push_data_t;
    if(pthread_create(&push_data_t, nullptr, Kafka_Producer::push_topic_t, &prd) != 0){
        std::cerr << "failed generate producer Thread " << std::endl; 
        perror("producer_threads");
        exit(1);
    }else{
        producer_thread_Manager.push_back(&push_data_t);
    }
    
    // ======= Consumer Part ========
    Kafka_Consumer cons(CONS_BROKER, CONS_TOPICS, CONS_GROUP_ID); 
    cons.set_kafka_conf();
    cons.gen_kafka_consumer(); 

    pthread_t pull_data_t;
    if(pthread_create(&pull_data_t, nullptr, Kafka_Consumer::pull_topic_t, &cons) != 0){
    std::cerr << "failed generate consumer Thread " << std::endl; 
        perror("consumer_threads");
        exit(1);
    }else{
        consumer_thread_Manager.push_back(&pull_data_t);
    }

    // ====== Graphic Process Part ========



    //=====================================
    while(run){ sleep(1); };

    //종료 코드 - Consumer, Producer Thread 종료까지 대기 
    for (pthread_t* prd_thread : producer_thread_Manager) {
        pthread_join(*prd_thread, nullptr);  
        std::cout << "Deallocate producer_thread : " << *prd_thread << std::endl;
    }
    producer_thread_Manager.clear();
    for (pthread_t* cons_thread : consumer_thread_Manager) {
        pthread_join(*cons_thread, nullptr); 
        std::cout << "deallocate consumer_thread : " << *cons_thread << std::endl;
    }
    consumer_thread_Manager.clear();

    return nullptr;

}