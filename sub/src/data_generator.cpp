#include "data_generator.h"
std::string Cam_Data_Generator::generate(){

    // 카메라 데이터 생성 부분
    // ....


    /* JSON request 메시지 생성 예시.. */
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    UUID uuid; 

    Rapid_Json_Handler json_handler;
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
        jsonObject.AddMember("type", Value(field.type.c_str(), json_handler.document.GetAllocator()), json_handler.document.GetAllocator());
        jsonObject.AddMember("optional", Value(field.optional), json_handler.document.GetAllocator());
        jsonObject.AddMember("field", Value(field.field.c_str(), json_handler.document.GetAllocator()), json_handler.document.GetAllocator());
        jsonArray.PushBack(jsonObject, json_handler.document.GetAllocator());
    }
    json_handler.add_member_p("/schema/fields", jsonArray);

    // Payload 
    json_handler.add_member_p("/payload/msg_uuid", uuid.generate_uuid());
    json_handler.add_member_p("/payload/sensor", "camera1");
    json_handler.add_member_p("/payload/msg_type", "detected_object");
    json_handler.add_member_p("/payload/data", "{\"cam_id\":\"123123\",\"detected_class\":\"bird\",\"xyxy\":\"[23.12, 32.32, 12.1, 30.1]\"}");
    json_handler.add_member_p("/payload/time_stamp", now_ms);
    
    // std::cout << "generate json : ";
    // json_handler.print_json();
    
    /* ************************************* */
    std::cout << "test" <<std::endl;
    return json_handler.get_json_string();

}

std::string IMU_Data_Generator::generate(){
    // IMU 데이터 생성 부분
    // ....

    /* JSON request 메시지 생성 예시.. */
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    UUID uuid; 

    Rapid_Json_Handler json_handler;
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
        jsonObject.AddMember("type", Value(field.type.c_str(), json_handler.document.GetAllocator()), json_handler.document.GetAllocator());
        jsonObject.AddMember("optional", Value(field.optional), json_handler.document.GetAllocator());
        jsonObject.AddMember("field", Value(field.field.c_str(), json_handler.document.GetAllocator()), json_handler.document.GetAllocator());
        jsonArray.PushBack(jsonObject, json_handler.document.GetAllocator());
    }
    json_handler.add_member_p("/schema/fields", jsonArray);

    // Payload 
    json_handler.add_member_p("/payload/msg_uuid", uuid.generate_uuid());
    json_handler.add_member_p("/payload/sensor", "IMU1");
    json_handler.add_member_p("/payload/msg_type", "imu_data");
    json_handler.add_member_p("/payload/data", "{\"imu_id\":\"456456\",\"acc\":\"[0.1, 0.1, 1.0]\"}");
    json_handler.add_member_p("/payload/time_stamp", now_ms);
    
    // std::cout << "generate json : ";
    // json_handler.print_json();
    
    /* ************************************* */
    // std::cout << "test" <<std::endl;
    return json_handler.get_json_string();

}