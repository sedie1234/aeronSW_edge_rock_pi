// only linux
// Jetson Orin CPU 코어 개수 : 12-core + 12 threads

// 서브보드 코드

#include <thread>
#include "my_rdkafka_producer.h"
#include "sig_handler.h"
#include "utils/utils.h"

const int MSG_SIZE = 1024;
const unsigned int FREQ = 1000000; //us (1,000,000 = 1sec)

//Producer Define --> 나중에 config 파일 읽어와서 넣는걸로 변경
const std::string PRD_BROKER = "192.168.0.205";
const std::string PRD_TOPIC = "sub0";

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<std::string> integ_vector;
std::vector<pthread_t*> producer_thread_Manager; 
Kafka_Producer set_prd_status(const std::string broker, std::string topic, unsigned int FREQ);
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
     * ======= Producer Part ======== 
     * ============================== */
    Kafka_Producer prd = set_prd_status(PRD_BROKER, PRD_TOPIC, FREQ); //broker, topic, frequency(u_sec), ... 추가
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
    // consumer_thread_Manager.clear();
    return nullptr;

}

Kafka_Producer set_prd_status(const std::string broker, std::string topic, unsigned int freq){
    Kafka_Producer prd(broker, topic, freq);
    prd.set_kafka_conf();
    prd.gen_kafka_producer();
    return prd;
}