#include "data_generator.h"
#include "cam_detect.h"

#include <string.h>

extern "C"{
    #include "imu.h"
}
std::string Cam_Data_Generator::generate(){

    // 카메라 데이터 생성 부분
    Cam_Detect cam_detect;
    object_detect_result_list cam_det_result=cam_detect.object_detect(); 

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

#if 1 //cam data


    Value cam_json(kArrayType);

    Value camObject(kObjectType);
    camObject.AddMember("cam_id","123",allocator); //AddMember : key-value

    Value detArray(kArrayType);

    for(int i=0; i<cam_det_result.count; i++){ // YU0324 count =0 일 경우: 1) frame capture 못한 경우(에러), 2) caputre 되었지만 인식된 객체 없는 경우(정상) => 객체 인식된 경우에만 보내도록?
            Value camObjects(kObjectType);
            object_detect_result *result= &(cam_det_result.results[i]);

            Value boxArray(kArrayType);
            boxArray.PushBack(result->box.left,allocator).PushBack(result->box.top,allocator).PushBack(result->box.right,allocator).PushBack(result->box.bottom, allocator);
            
            camObjects.AddMember("detected_class",result->cls_id,allocator);
            camObjects.AddMember("xyxy",boxArray,allocator);
            camObjects.AddMember("prop",result->prop,allocator);
            detArray.PushBack(camObjects,allocator);
                
            
    }
    camObject.AddMember("detect_object",detArray,allocator);

    cam_json.PushBack(camObject,allocator);

    
#endif
    // Payload 
    json_handler.add_member_p("/payload/msg_uuid", uuid.generate_uuid());
    json_handler.add_member_p("/payload/sensor", "camera1");
    json_handler.add_member_p("/payload/msg_type", "detected_object");
    // json_handler.add_member_p("/payload/data", "{\"cam_id\":\"123123\",\"detected_class\":\"bird\",\"xyxy\":\"[23.12, 32.32, 12.1, 30.1]\"}");
    json_handler.add_member_p("/payload/data", cam_json);
    json_handler.add_member_p("/payload/time_stamp", now_ms);
    
    

    // std::cout << "generate json : ";
    // json_handler.print_json();
    
    /* ************************************* */
    std::cout << "test" <<std::endl;
    return json_handler.get_json_string();

}

std::string IMU_Data_Generator::generate(){
    // IMU 데이터 생성 부분
    char imu_buffer_15[MAX_READ_SIZE]={0};
    char imu_buffer_16[MAX_READ_SIZE]={0};
#if 1 // 쿼터니언 값만 필요할 경우

    switch_sensor_mode(16);
    read_response_multiple(3, imu_buffer_16);

#elif 0 // 모든 데이터 필요한 경우
        // --- Step 1: Switch to ss=15 ---
        switch_sensor_mode(15);
        read_response_multiple(3, imu_buffer_15);  // 5줄 정도 읽음


        // --- Step 2: Switch to ss=16 ---
        switch_sensor_mode(16);
        read_response_multiple(3,imu_buffer_16);  // 쿼터니언 3줄 정도 읽음
#endif

    if(imu_buffer_15!=nullptr){
        std::cout << "imu_data 15 = "<< imu_buffer_15<< std::endl;
    }
    if(imu_buffer_16!=nullptr){
        std::cout << "imu_data 16 = "<< imu_buffer_16<< std::endl;
    }
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
    imu_json.AddMember("data" ,Value(imu_buffer_16,allocator), allocator);

    // Payload 
    json_handler.add_member_p("/payload/msg_uuid", uuid.generate_uuid());
    json_handler.add_member_p("/payload/sensor", "IMU1");
    json_handler.add_member_p("/payload/msg_type", "imu_data");
    // json_handler.add_member_p("/payload/data", "{\"imu_id\":\"456456\",\"acc\":\"[0.1, 0.1, 1.0]\"}");
    json_handler.add_member_p("/payload/data", imu_json);
    json_handler.add_member_p("/payload/time_stamp", now_ms);
    
    // std::cout << "generate json : ";
    // json_handler.print_json();
    
    /* ************************************* */
    // std::cout << "test" <<std::endl;
    return json_handler.get_json_string();

}
