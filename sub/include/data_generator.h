#ifndef _DATA_GENERATOR_H
#define _DATA_GENERATOR_H

#include <iostream>
#include <memory>

//메시지 생성 관련 Header
#include "rapid_json_handler.h"
#include "utils/uuid.h"

#include <chrono>


//카프카 Connector Sink DB 저장을 위한 Schema 폼
struct Fields_Info{
    std::string type;
    bool optional;
    std::string field;
};

//데이터 생성 클래스 공통 인터페이스
class IData_Generator{
    public:
        virtual ~IData_Generator() = default;
        virtual std::string generate() = 0;

        // Getter 함수 
        virtual std::string get_broker() const = 0;
        virtual std::string get_topic() const = 0;
        virtual unsigned int get_freq() const = 0;

};

//카메라 데이터 생성
class Cam_Data_Generator : public IData_Generator{
        
    private: //Destination
        const std::string PRD_BROKER = "192.168.0.205";
        const std::string PRD_TOPIC = "sub0";
        const unsigned int FREQ = 1000000;

    public:
        std::string generate() override;
        std::string get_broker() const override { return PRD_BROKER; }
        std::string get_topic() const override { return PRD_TOPIC; }
        unsigned int get_freq() const override { return FREQ; }

};

//IMU 데이터 생성
class IMU_Data_Generator : public IData_Generator{

    private: //Destination
        const std::string PRD_BROKER = "192.168.0.205";
        const std::string PRD_TOPIC = "sub0_imu";
        const unsigned int FREQ = 1000000;

    public:
        std::string generate() override;
        std::string get_broker() const override { return PRD_BROKER; }
        std::string get_topic() const override { return PRD_TOPIC; }
        unsigned int get_freq() const override { return FREQ; }

};

#endif