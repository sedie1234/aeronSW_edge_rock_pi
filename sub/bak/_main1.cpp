// only linux
// Jetson Orin CPU 코어 개수 : 12-core + 12 threads


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
// const std::string CONS_TOPIC1 = "sub1_topic_test";
// const std::string CONS_TOPIC2 = "sub2_topic_test";
const std::string CONS_BROKER = "127.0.0.1";
const std::string CONS_GROUP_ID = "main";
const std::vector<std::string> SUB_BOARD_LIST = {"sub1","sub2"}; //

//Test 용
/* 장비 구성 -- 
* 서브보드 2개로 구성. 각각 Cam, Lidar 전송 한다고 가정
*/



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

std::vector<Kafka_Producer*> producer_Pool; 
std::vector<Kafka_Producer*> consumer_Pool;
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

    // 실행중.....
    for(std::string line; run && getline(std::cin, line); ){

        std::cout << "rrr" << std::endl;
        
        if(line == "li" or line=="list"){
            std::cout << "deving..." << std::endl;

        }else if(line == "restart"){
            std::cout << "terminate server" << std::endl;       
            //raise(SIGUSR1);

        }else if(line == "exit"){
            std::cout << "terminate server" << std::endl;
            raise(SIGTERM);
            pthread_detach(th_handler); //프로세스 로부터 분리
            break;
        }else{   
        }
    }

    //종료 코드.
    pthread_join(th_handler, nullptr); //th_handler 종료 대기
    for (Kafka_Producer* p : producer_Pool) {
        std::cout << "Deallocate producer memory : " << p << std::endl;
        delete p;
    }
    // for (Kafka_Producer* c : consumer_Pool) {
    //     std::cout << "Deallocate consumer memory : " << c << std::endl;
    //     delete c;
    // }

    for (pthread_t* prd_thread : producer_thread_Manager) {
        std::cout << "Deallocate producer_thread : " << *prd_thread << std::endl;
        pthread_detach(*prd_thread);
    }
    // for (pthread_t* cons_thread : consumer_thread_Manager) {
    //     std::cout << "deallocate consumer_thread" << cons_thread << std::endl;
    //     pthread_detach(*cons_thread);
    // }

    pthread_mutex_destroy(&mutex);

}

void* thread_handler(void* data){
    (void)data;

    // Producer Part
    // producer instance 생성
    Kafka_Producer* prd = new Kafka_Producer(PRD_BROKER, PRD_TOPIC);
    prd->set_kafka_conf();
    prd->gen_kafka_producer();
    
    // 메인보드에서 Request topic 으로 메세지 요청하는 쓰레드 생성. (아마 1개 정도에서 더 늘어날일 없을듯)
    pthread_t* push_data_t = new pthread_t; 
    if(pthread_create(push_data_t, nullptr, Kafka_Producer::push_topic_t, prd) != 0){
        std::cerr << "failed generate req2sub Thread " << std::endl; 
        perror("producer_threads");
        exit(1);
    }else{
        producer_Pool.push_back(prd);
        producer_thread_Manager.push_back(push_data_t);
        delete push_data_t;
    }

    // Subscribe 되어있는 Topic 으로부터 데이터를 Consume 하는 쓰레드 생성 --> 일단 한개만 생성해서 해보는걸로..
    
    //구독할 Topics 준비.. Topics 는 vector 형태로,,  
    std::vector<std::string> topics_list;
    for(std::string sub_board_name : SUB_BOARD_LIST){
        sub_board_name+="_topic_test";
        topics_list.push_back(sub_board_name);
    }
    
    Kafka_Consumer* cons = new Kafka_Consumer(topics_list, CONS_BROKER, CONS_GROUP_ID);
    

    // pthread_t* pull_dat_t = new pthread_t; 
    // if(pthread_create(pull_dat_t, nullptr, cons_pull_topic, nullptr) != 0){
    //     std::cerr << "failed generate req2sub Thread " << std::endl; 
    //     perror("producer_threads");
    //     delete pull_dat_t;
    //     raise(SIGTERM);

    // }else{
    //     consumer_thread_Manager.push_back(*pull_dat_t);
    //     delete pull_dat_t;
    // }

    /* 생성된 모든 쓰레드 분리 실행 */
    // Producer Thread 실행
    // for (pthread_t prd_thread : producer_thread_Manager) {
    //     pthread_detach(prd_thread); // Thread 분리 실행
    // }
    // Consumer Thread 실행
    // for (pthread_t cons_thread : consumer_thread_Manager) {
    //     pthread_detach(cons_thread); // Thread 분리 실행
    // }
}