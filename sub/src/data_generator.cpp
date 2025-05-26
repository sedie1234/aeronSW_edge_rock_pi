#include "data_generator.h"
#include "cam_detect.h"
// #include <rapidjson/writer.h>
// #include <rapidjson/stringbuffer.h>

#include <string.h>
#include <ctime>

#include <thread>

extern "C"{
    #include "imu.h"
}

void Cam_Data_Generator::run_camera_thread(){
    std::thread([this](){
        Cam_Detect cam_detect;
        TimeData<object_detect_result_list> td;

        while(1){
#if TIME
            object_detect_result_list od_data=cam_detect.object_detect();

            if(!od_data.count) continue; //객체 감지했을때만 push

            //time
            td.timestamp=std::chrono::system_clock::now();
            td.data=od_data;
             //push 
            data_queue_->push(td,"cam");
            // std::cout <<"cam push time :" <<data_queue_.timestamp  << std::endl;
         
#else
            object_detect_result_list od_data=cam_detect.object_detect();

            if(!od_data.count) continue; //객체 감지했을때만 push

             //push 
            data_queue_->push(od_data);
#endif
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
}

void IMU_Data_Generator::run_imu_thread(){
    std::thread([this](){
        while(1){
            //imu 
            
            char imu_buffer[MAX_READ_SIZE]={0};
            TimeData<char*> td;
        #if 1 // 쿼터니언 값만 필요할 경우

            switch_sensor_mode(16);
            read_response_multiple(3, imu_buffer);
            if(imu_buffer!=nullptr){
#if TIME
            td.timestamp=std::chrono::system_clock::now();
            td.data=imu_buffer;

            data_queue_imu->push(td,"imu");
            // std::cout <<"cam push time :" <<data_queue_imu.timestamp  << std::endl;
#else
                data_queue_imu->push(imu_buffer);
#endif

            }

        #elif 0 // 모든 데이터 필요한 경우
                char imu_buffer_15[MAX_READ_SIZE]={0};
                // --- Step 1: Switch to ss=15 ---
                switch_sensor_mode(15);
                read_response_multiple(3, imu_buffer_15);  // 5줄 정도 읽음


                // --- Step 2: Switch to ss=16 ---
                switch_sensor_mode(16);
                read_response_multiple(3,imu_buffer_16);  // 쿼터니언 3줄 정도 읽음


                if(imu_buffer_15!=nullptr){
                    std::cout << "imu_data 15 = "<< imu_buffer_15<< std::endl;
                }
        #endif

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
}

//push, pop time print
void print_current_time(auto pushed,const std::string& label){//push, pop 시간 출력 , 0.1s 주기일때 push, pop 거의바로 이루어지는중
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(pushed.time_since_epoch()) % 1000;
    std::time_t t_c=std::chrono::system_clock::to_time_t(pushed);
            
    auto now = std::chrono::system_clock::now();
    auto n_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::cout << "##########[" << label <<"PUSHED @] "
                    << std::put_time(std::localtime(&t_c), "%F %T")
                    << "." << std::setfill('0') << std::setw(3) << ms.count()
                    << std::endl;

    std::cout << "###########[" << label << "POPPED] "
                   << std::put_time(std::localtime(&now_c), "%F %T")
                    << "." << std::setfill('0') << std::setw(3) << n_ms.count()
                    << std::endl;
}                   

std::string Cam_Data_Generator::generate(){

#if TIME
    //pop
    TimeData<object_detect_result_list> cam_det_result;
    if(!data_queue_->pop(cam_det_result,"cam")){
        return "object null\n";
    }
    
    auto pushed =cam_det_result.timestamp;
    print_current_time(pushed,"CAM");

#else 
        //pop
    object_detect_result_list cam_det_result;
    if(!data_queue_->pop(cam_det_result)){
        return "object null";
    }
#endif
    /* JSON request 메시지 생성 예시.. */
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    UUID uuid; 

    Rapid_Json_Handler json_handler;
    Document::AllocatorType& allocator =json_handler.document.GetAllocator();
    //schema
    json_handler.add_member_p("/schema/type", "struct");
    std::vector<Fields_Info> fields = {
        //type      optional    field
        {"string",  false,      "msg_uuid"},
        {"string",  false,      "group"},
        {"string",  false,      "sensor"},
        {"string",  false,      "msg_type"},
        {"string",  false,      "data"},
        {"int64",   false,      "time_stamp"}
    };

    Value jsonArray(kArrayType); 

    for (const auto& field : fields) {
        Value jsonObject(kObjectType);
        jsonObject.AddMember("type", Value(field.type.c_str(), allocator), allocator);
        jsonObject.AddMember("optional", Value(field.optional), allocator);
        jsonObject.AddMember("field", Value(field.field.c_str(), allocator), allocator);
        jsonArray.PushBack(jsonObject, allocator);
    }
    json_handler.add_member_p("/schema/fields", jsonArray);

    //output:  "{\"cam_id\":\"123\",\"detect_object\":[\"{\\\"detected_class\\\":66,\\\"xyxy\\\":[6,9,638,469],\\\"prop\\\":0.34092748165130615}\",\"{\\\"detected_class\\\":67,\\\"xyxy\\\":[14,10,638,469],\\\"prop\\\":0.25956979393959045}\"]}"
    Value cam_json(kArrayType);

    Value camObject(kObjectType);
    camObject.AddMember("cam_id","123",allocator); //AddMember : key-value

    Value detArray(kArrayType);
#if TIME
    for(int i=0; i<cam_det_result.data.count; i++){ // YU0324 count =0 일 경우: 1) frame capture 못한 경우(카메라 연결 에러), 2) 인식된 객체 없는 경우(정상) => 객체 인식된 경우에만 보내도록?
        //   "{
            printf("cam_det_result.data.count= %d\n",cam_det_result.data.count );
            Value camObjects(kObjectType);
            object_detect_result *result= &(cam_det_result.data.results[i]);
#else
    for(int i=0; i<cam_det_result.count; i++){ // YU0324 count =0 일 경우: 1) frame capture 못한 경우(카메라 연결 에러), 2) 인식된 객체 없는 경우(정상) => 객체 인식된 경우에만 보내도록?
        //   "{
            Value camObjects(kObjectType);
            object_detect_result *result= &(cam_det_result.results[i]);
#endif

            // []
            Value boxArray(kArrayType);
            int box[] = { result->box.left, result->box.top, result->box.right, result->box.bottom };
            for (int v : box) {
                std::string v_str=std::to_string(v);
                boxArray.PushBack(Value(v_str.c_str(),allocator), allocator);
            }

            camObjects.AddMember("detected_class",result->cls_id,allocator);
            camObjects.AddMember("xyxy",boxArray,allocator);
            camObjects.AddMember("prop",result->prop,allocator);
                
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            camObjects.Accept(writer);
            std::string cam_objstring=buffer.GetString();

            detArray.PushBack(Value(cam_objstring.c_str(), allocator),allocator);
            // }"
    }
    // []
    camObject.AddMember("detect_object",detArray,allocator);

    StringBuffer buffer_fin;
    Writer<StringBuffer> writer(buffer_fin);
    camObject.Accept(writer);
    std::string cam_string=buffer_fin.GetString();

    cam_json.PushBack(Value(cam_string.c_str(),allocator),allocator); //json 형태를 string으로 보냄

    // Payload 
    json_handler.add_member_p("/payload/msg_uuid", uuid.generate_uuid());
    json_handler.add_member_p("/payload/group", "sub0");// 후에 수정
    json_handler.add_member_p("/payload/sensor", "camera1");
    json_handler.add_member_p("/payload/msg_type", "detected_object");

    // json_handler.add_member_p("/payload/data", "{\"cam_id\":\"123123\",\"detected_class\":\"bird\",\"xyxy\": }");
    json_handler.add_member_p("/payload/data",cam_json);

    json_handler.add_member_p("/payload/time_stamp", now_ms);
    
    

    // std::cout << "generate json : ";
    // json_handler.print_json();
    
    /* ************************************* */
    std::cout << "test" <<std::endl;
    return json_handler.get_json_string();

}

std::string IMU_Data_Generator::generate(){
#if TIME
    //pop
    TimeData<char*> imu_buffer_16;
    if(!data_queue_imu->pop(imu_buffer_16,"imu")){
        return "imu null\n";
    }

    auto pushed =imu_buffer_16.timestamp;
    print_current_time(pushed,"IMU");


#else 
    char* imu_buffer_16=new char[MAX_READ_SIZE];
    //pop
    if(!data_queue_imu->pop(imu_buffer_16)){
        return "imu null";
    }
#endif
    /* JSON request 메시지 생성 예시.. */
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    UUID uuid; 

    Rapid_Json_Handler json_handler;
    Document::AllocatorType& allocator =json_handler.document.GetAllocator();

    //schema
    json_handler.add_member_p("/schema/type", "struct");
    std::vector<Fields_Info> fields = {
        //type      optional    field
        {"string",  false,      "msg_uuid"},
        {"string",  false,      "group"},
        {"string",  false,      "sensor"},
        {"string",  false,      "msg_type"},
        {"string",  false,      "data"},
        {"int64",   false,      "time_stamp"}
    };

    Value jsonArray(kArrayType); 

    for (const auto& field : fields) {
        Value jsonObject(kObjectType);
        jsonObject.AddMember("type", Value(field.type.c_str(), allocator), allocator);
        jsonObject.AddMember("optional", Value(field.optional), allocator);
        jsonObject.AddMember("field", Value(field.field.c_str(), allocator), allocator);
        jsonArray.PushBack(jsonObject, allocator);
    }
    json_handler.add_member_p("/schema/fields", jsonArray);


    // imu data
    Value imu_json(kObjectType);// key-value
    
    imu_json.AddMember("imu_id", "123456",allocator);
    #if 0
    imu_json.AddMember("data" ,Value(imu_buffer_15,allocator), allocator);
    #endif
#if TIME 
    //"    1.00     0.01     0.03     0.01\r" imu data split 
    std::replace(imu_buffer_16.data, imu_buffer_16.data + std::strlen(imu_buffer_16.data), '\\', ' ');
    
    std::istringstream stream(imu_buffer_16.data);
#else
    //"    1.00     0.01     0.03     0.01\r" imu data split 
    std::replace(imu_buffer_16, imu_buffer_16 + std::strlen(imu_buffer_16), '\\', ' ');
    
    std::istringstream stream(imu_buffer_16);
#endif
    std::vector<std::string> result;
    std::string token;
    
    while (stream >> token) {  // 연속된 공백도 자동 무시됨
        // \r 같은 특수문자 제거
        token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
        result.push_back(token);
    }
    Value data_arr(kArrayType);
    for (const auto& tok : result) {
           
        std::cout << tok<< std::endl;
        data_arr.PushBack(Value(tok.c_str(),allocator),allocator);
    }


    imu_json.AddMember("data",data_arr,allocator);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    imu_json.Accept(writer);
    std::string imu_string=buffer.GetString(); // json형태를 string으로 전달

    // Payload 
    json_handler.add_member_p("/payload/msg_uuid", uuid.generate_uuid());
    json_handler.add_member_p("/payload/group", "sub0");// 후에 수정
    json_handler.add_member_p("/payload/sensor", "imu");
    json_handler.add_member_p("/payload/msg_type", "imu_data");
    // json_handler.add_member_p("/payload/data", "{\"imu_id\":\"456456\",\"acc\":\"[0.1, 0.1, 1.0]\"}");
    json_handler.add_member_p("/payload/data", imu_string); 
    json_handler.add_member_p("/payload/time_stamp", now_ms);
    
    // std::cout << "generate json : ";
    // json_handler.print_json();
    
    /* ************************************* */
    // std::cout << "test" <<std::endl;
    return json_handler.get_json_string();

}
