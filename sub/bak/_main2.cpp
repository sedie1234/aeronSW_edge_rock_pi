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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// std::vector<Kafka_Producer*> producer_Pool; 
// std::vector<Kafka_Producer*> consumer_Pool;
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
    for (pthread_t* prd_thread : producer_thread_Manager) {
        pthread_join(*prd_thread, nullptr);  //push_thread(producer) 종료 대기
        std::cout << "Deallocate producer_thread : " << *prd_thread << std::endl;
    }
    for (pthread_t* cons_thread : consumer_thread_Manager) {
        pthread_join(*cons_thread, nullptr); //pull_thread(consumer) 종료 대기
        std::cout << "deallocate consumer_thread" << *cons_thread << std::endl;
    }

    pthread_join(th_handler, nullptr); //마지막으로 th_handler 종료 대기.
    pthread_mutex_destroy(&mutex);

}

void* thread_handler(void* data){
    (void)data;

    // ====== Producer Part ======
    // producer instance 생성
    Kafka_Producer prd(PRD_BROKER, PRD_TOPIC);
    prd.set_kafka_conf();
    prd.gen_kafka_producer();
    
    // pthread_t* push_data_t = new pthread_t; 메인보드에서 Request topic 으로 메세지 요청하는 쓰레드 생성. (아마 1개 정도에서 더 늘어날일 없을듯)
    pthread_t push_data_t;
    if(pthread_create(&push_data_t, nullptr, Kafka_Producer::push_topic_t, &prd) != 0){
        std::cerr << "failed generate producer Thread " << std::endl; 
        perror("producer_threads");
        exit(1);
    }else{
        //producer_Pool.push_back(&prd);
        producer_thread_Manager.push_back(&push_data_t);
    }
    
    // ======= Consumer Part ========
    //구독할 Topic 리스트 준비, sub1, sub2, sub3 ... 여러개 구독해야하니 vector 형태로. 
    std::vector<std::string> topics_list;
    for(std::string sub_board_name : SUB_BOARD_LIST){
        sub_board_name+="_topic_test";
        topics_list.push_back(sub_board_name);
    }
    //Kafka_Consumer* cons = new Kafka_Consumer(topics_list, CONS_BROKER, CONS_GROUP_ID); // --> Consumer Thread 별로 나눠서 관리하려고 했는데 꼭 그렇게 할 필요 없을것 같음
    //Consumer Instance 생성
    Kafka_Consumer cons; //test, req_test, sub1_topic_test, sub2_topic_test
    //Kafka_Consumer cons(topics_list, CONS_BROKER, CONS_GROUP_ID); 
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


    // Producer Thread 데모 실행
    for (pthread_t* prd_thread : producer_thread_Manager) {
        pthread_detach(*prd_thread); // Thread 분리 실행
    }
    // Consumer Thread 데모 실행
    for (pthread_t* cons_thread : consumer_thread_Manager) {
        pthread_detach(*cons_thread); // Thread 분리 실행
    }

    while(run){ // th_handler 끝나면 prd, cons 객체 사라지기때문에 이 쓰레드 유지.
        //std::cout <<"handler run " << run <<std::endl;
        sleep(1);
    };

    return nullptr;

}