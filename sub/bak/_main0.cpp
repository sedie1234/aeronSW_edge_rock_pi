// only linux
// Jetson Orin CPU 코어 개수 : 12-core + 12 threads


#include <thread>

#include "my_rdkafka_producer_lib.h"
#include "my_rdkafka_consumer_lib.h"
#include "sig_handler.h"
#include "utils.h"


//Producer Define
const std::string BROKER_IP = "127.0.0.1";
const std::string TOPIC = "req_topic";
const int MSG_SIZE = 1024;

//Consumer Define
//Test 용
/* 장비 구성 -- 
* 장비 Type(cam, lidar, ... ), 갯수, 
* cam 2개, lidar 1개로 가정. -> cam_t 2개 생성 + lidar_t 1개 생성 => 총 3개 Thread 생성.
*/
// 일단 임시로 대충 설정
const std::vector<std::vector<std::string>> DEVICE_SET = {{"cam","2"},{"lidar","1"}};

//
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//std::vector<Kafka_Producer> prd_Pool;
std::vector<Kafka_Producer> prd_Pool; 
std::vector<pthread_t> producer_thread_Manager; 
std::vector<pthread_t> consumer_thread_Manager; 

void* thread_handler(void* data);
void* prd_push_req_topic(void* data);
void* cons_pull_topic(void* data);
void* cons_pull_lid_topic(void* data);

int main(){
    setup_sig_handler();

    pthread_t th_handler;
    if(pthread_create(&th_handler, nullptr, thread_handler, nullptr) != 0){
        std::cerr << "failed generate Thread " << std::endl; 
        return -1;    
    }

    // 실행중.....
    char msg[MSG_SIZE] = {0};
    while(run){
        memset(msg, 0, MSG_SIZE);
        std::cin >> msg;
        if((std::string)msg == "li" or (std::string)msg=="list"){
            std::cout << "deving..." << std::endl;

        }else if((std::string)msg == "exit"){
            std::cout << "terminate server" << std::endl;
            pthread_detach(th_handler); 
            std::cout << "before sigterm run : " << run << std::endl;
            raise(SIGTERM);
            std::cout << "after sigterm run : " << run << std::endl;
            //break;
        }else{   
        }
    }

    //종료 코드.
    pthread_join(th_handler, nullptr); //th_handler 종료 대기
    
    for (pthread_t prd_thread : producer_thread_Manager) {
        std::cout << "deallocate producer_thread" << prd_thread << std::endl;
        pthread_detach(prd_thread);
    }
    for (pthread_t cons_thread : consumer_thread_Manager) {
        std::cout << "deallocate consumer_thread" << cons_thread << std::endl;
        pthread_detach(cons_thread);
    }

    pthread_mutex_destroy(&mutex);

}


void* thread_handler(void* data){
    (void)data;

    // 메인보드에서 Request topic 으로 메세지 요청하는 쓰레드 생성. (1개면 되겠지?)
    pthread_t* push_data_t = new pthread_t; 
    Kafka_Producer* prd = new Kafka_Producer(BROKER_IP, TOPIC);
    if(pthread_create(push_data_t, nullptr, prd_push_req_topic, nullptr) != 0){
        std::cerr << "failed generate req2sub Thread " << std::endl; 
        perror("producer_threads");
        delete prd;
        delete push_data_t;
        raise(SIGTERM);
    }else{
        producer_thread_Manager.push_back(*push_data_t);
        delete push_data_t;
    }

    // Subscribe 되어있는 Topic 으로부터 데이터를 Consume 하는 쓰레드 생성.
    /* Subscribe는 Config 파일을 읽어와서 실행? */

    for(std::vector<std::string> devices_info : DEVICE_SET){
        std::string device = devices_info[0];
        std::string num = devices_info[1];
        std::cout <<"vector : " << devices_info << "curr dev : " << device << "num : " << num << std::endl;
    }

    pthread_t* pull_dat_t = new pthread_t; 
    

    if(pthread_create(pull_dat_t, nullptr, cons_pull_topic, nullptr) != 0){
        std::cerr << "failed generate req2sub Thread " << std::endl; 
        perror("producer_threads");
        
        delete pull_dat_t;
        raise(SIGTERM);

    }else{
        prd_Pool.push_back(*prd);
        consumer_thread_Manager.push_back(*pull_dat_t);
        delete prd;
        delete pull_dat_t;
    }

    /* 생성된 모든 쓰레드 분리 실행 */
    // Producer Thread 실행
    for (pthread_t prd_thread : producer_thread_Manager) {
        pthread_detach(prd_thread); // Thread 분리 실행
    }
    // Consumer Thread 실행
    for (pthread_t cons_thread : consumer_thread_Manager) {
        pthread_detach(cons_thread); // Thread 분리 실행
    }
}

/* request to sub_board Producer 생성. (1초에 한번씩 req message 를 m_req_topic 에 전달. */
void* prd_push_req_topic(void* data){

    while(true){
        std::cout << "req" << std::endl;
        sleep(1);
    }
    
}

// 데이터 어떻게 들어올지 모르고 다르게 처리해야 할 수 도 있으니 일단 나눠놓음
/* cam topic  */
void* cons_pull_topic(void* data){

    while(true){
        std::cout << "pull" << std::endl;
        sleep(1);
    }

}

void* cons_pull_cam_topic(void* data){

    while(true){
        std::cout << "cam" << std::endl;
        sleep(1);
    }

}

/* lidar topic */
void* cons_pull_lid_topic(void* data){

    while(true){
        std::cout << "lidar" << std::endl;
        sleep(1);
    }

}