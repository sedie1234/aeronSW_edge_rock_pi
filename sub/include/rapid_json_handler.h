#ifndef R_JSON_HANDLER_H
#define R_JSON_HANDLER_H

//Rapid_Json handler
//https://github.com/miloyip/nativejson-benchmark --> 참고 JSON 벤치마크 (Rapid JSON 사용)

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map> // 해쉬테이블로 구현된 자료구조, 중복데이터 허용 안함. 
#include <map> 
#include <type_traits>
#include <variant>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h" 
#include "rapidjson/pointer.h"
#include "rapidjson/rapidjson.h"


using namespace rapidjson;
using json_value = std::variant<std::string, const char*, int, bool, double, float, std::vector<std::string>>;

class Rapid_Json_Handler{
    public:
    Document document;
    Rapid_Json_Handler();
    template <typename T> 
    void add_member_p(const std::string key, const T &value);
    json_value json_parsing(const char* msg_payload, const char* key);
    std::string get_json_string() const; 
    Value* get_value(const std::string& key);
    void print_json() const;

    // void parse_from_file(const std::string& filename);
    // void save_to_file(const std::string& filename) const;
};

#include "rapid_json_handler.inl" // inline 파일 마지막에 포함

#endif