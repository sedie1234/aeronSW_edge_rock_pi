#ifndef _DATA_GENERATOR_H
#define _DATA_GENERATOR_H

#include <iostream>
#include <memory>

//메시지 생성 관련 Header
#include "rapid_json_handler.h"
#include "utils/uuid.h"

#include <chrono>

#include "sensor_queue.h"
#include "yolov8.h"

#define TIME 1
#if TIME
template<typename T>
struct TimeData{ // push, pop 되는 시간 추가 data
    std::chrono::system_clock::time_point timestamp;
    T data;
};

#endif

//카프카 Connector Sink DB 저장을 위한 Schema 폼
struct Fields_Info{
    std::string type;
    bool optional;
    std::string field;
};

static std::string SUB_GROUP="sub";

//데이터 생성 클래스 공통 인터페이스
class IData_Generator{
    public:
        virtual ~IData_Generator() = default;
        virtual std::string generate() = 0;

        // virtual int init(int argc, char** argv);
        static void gen_init(char** argv);

        // Getter 함수 
        virtual std::string get_broker() const = 0;
        virtual std::string get_topic() const = 0;
        virtual unsigned int get_freq() const = 0;

};

//카메라 데이터 생성
class Cam_Data_Generator : public IData_Generator{
        

    public:
    #if TIME
        Cam_Data_Generator(){ //생성자
            data_queue_=std::make_shared<DataQueue<TimeData<object_detect_result_list>>>();
        }       
    #else
        Cam_Data_Generator(){ //생성자
            data_queue_=std::make_shared<DataQueue<object_detect_result_list>>();
        }
    #endif
        std::string generate() override;
        std::string get_broker() const override { return PRD_BROKER; }
        std::string get_topic() const override { return PRD_TOPIC; }
        unsigned int get_freq() const override { return FREQ; }

        void run_camera_thread();

    private: //Destination
        const std::string PRD_BROKER = "192.168.0.205";
        const std::string PRD_TOPIC = "sub0";
        const unsigned int FREQ = 100000; //0.1s 1s=1000000us
#if TIME
        std::shared_ptr<DataQueue<TimeData<object_detect_result_list>>> data_queue_;
#else
        std::shared_ptr<DataQueue<object_detect_result_list>> data_queue_;
#endif

};

//IMU 데이터 생성
class IMU_Data_Generator : public IData_Generator{

    private: //Destination
        const std::string PRD_BROKER = "192.168.0.205";
        const std::string PRD_TOPIC = "sub0_imu";
        const unsigned int FREQ = 100000;//0.1s

#if TIME
        std::shared_ptr<DataQueue<TimeData<char*>>> data_queue_imu;
#else
        std::shared_ptr<DataQueue<char*>> data_queue_imu;
#endif
    public:
#if TIME
        IMU_Data_Generator(){ //생성자
            data_queue_imu=std::make_shared<DataQueue<TimeData<char*>>>();
        }
#else
        IMU_Data_Generator(){ //생성자
            data_queue_imu=std::make_shared<DataQueue<char*>>();
        }
#endif
        std::string generate() override;
        std::string get_broker() const override { return PRD_BROKER; }
        std::string get_topic() const override { return PRD_TOPIC; }
        unsigned int get_freq() const override { return FREQ; }

        void run_imu_thread();


};

#endif