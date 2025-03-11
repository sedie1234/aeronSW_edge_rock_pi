// only linux
// Jetson Orin CPU 코어 개수 : 12-core + 12 threads

// 서브보드 코드

#include <thread>
#include "my_rdkafka_producer.h"
#include "data_generator.h"
#include "sig_handler.h"
#include "utils/utils.h"

const int MSG_SIZE = 1024;
const unsigned int FREQ = 1000000; //us (1,000,000 = 1sec)

//Producer Define --> 나중에 config 파일 읽어와서 넣는걸로 변경
const std::string PRD_BROKER = "192.168.0.205";
const std::string PRD_TOPIC = "sub0";

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<std::string> integ_vector;
std::vector<Kafka_Producer*> producer_Pool; 
std::vector<Thread_Args*> thread_args_Pool; 

std::vector<pthread_t*> producer_thread_Manager; 
Kafka_Producer set_prd_status(const std::string broker, std::string topic, unsigned int FREQ);

void* thread_handler(void* data);

int main(){

    setup_sig_handler();
    auto th_handler = std::unique_ptr<pthread_t, PThreadDeleter>(new pthread_t);    

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

    //종료 코드
    producer_Pool.clear();
    thread_args_Pool.clear();
    producer_thread_Manager.clear();

    pthread_mutex_destroy(&mutex);

    return 0;
}

void* thread_handler(void* data){
    (void)data;

    /* ============================== 
     * ======= Producer Part ======== 
     * ============================== */
    
    
    auto cam_data = std::make_shared<Cam_Data_Generator>(); //카메라 데이터 생성 객체
    auto imu_data = std::make_shared<IMU_Data_Generator>(); //IMU 데이터 생성 객체

    std::vector<std::shared_ptr<IData_Generator>> data_generators; //생성된 객체를 Vector 에 넣음
    data_generators.push_back(cam_data);
    data_generators.push_back(imu_data);

    for (const auto& generator : data_generators) { //Vector 안에 있는 객체별로 생성 + 전송 하는 코드
        
        // 각 센서별로 producer instance 생성, 
        Kafka_Producer* prd = new Kafka_Producer(generator->get_broker(), generator->get_topic(), generator->get_freq());
        Thread_Args* ta = new Thread_Args{generator, prd};
        pthread_t* push_data_t = new pthread_t; 
        if(pthread_create(push_data_t, nullptr, Kafka_Producer::push_topic_t, ta) != 0){
            std::cerr << "failed generate producer Thread " << std::endl; 
            perror("producer_threads");
            exit(1);
        }else{
            producer_Pool.push_back(prd);
            thread_args_Pool.push_back(ta);
            producer_thread_Manager.push_back(push_data_t);
            delete push_data_t;
        }
    }

    /* 생성된 모든 쓰레드 분리 실행 */
    // Producer Thread 실행
    for (pthread_t *prd_thread : producer_thread_Manager) {
        pthread_detach(*prd_thread);
    }


    while(run){ sleep(1); }


    //thread_handler 종료
    return nullptr;
}





