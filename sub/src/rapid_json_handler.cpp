#include "rapid_json_handler.h"

Rapid_Json_Handler::Rapid_Json_Handler() {
    document.SetObject();
}

json_value Rapid_Json_Handler::json_parsing(const char* msg, const char* key){
    document.Parse(msg);
    if(const Value* val = Pointer(key).Get(document)){ //ex) key : /schema/type
        if(val->IsString()){
            return std::string(val->GetString());
        }else if(val->IsInt()){
            return int(val->GetInt());
        }else if(val->IsBool()){
            return bool(val->GetBool());
        }else if(val->IsDouble()){
            return double(val->GetDouble());
        }else if(val->IsFloat()){
            return float(val->GetFloat());
        }else if(val->IsArray()){
            std::vector<std::string> data;
            for (const auto& v : val->GetArray()) { //배열 String 만 가능
                data.push_back(v.GetString());
            }
            return data;
        }else{
            std::cout << "whatasdasdad" << std::endl;
            std::cout << std::string(val->GetString()) << std::endl;
        }
    }
    return 1;
}

std::string Rapid_Json_Handler::get_json_string() const {
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}
    
Value* Rapid_Json_Handler::get_value(const std::string& key) {
    if (document.HasMember(key.c_str())) {
        return &document[key.c_str()];
    }
    return nullptr;
}

void Rapid_Json_Handler::print_json() const {
    std::cout << get_json_string() << std::endl;
}



// JSON 파일 저장은 지금 안쓸꺼 같으니 주석 처리... 필요하면 해제 

// void Rapid_Json_Handler::parse_from_file(const std::string& filename) {
//     std::ifstream ifs(filename);
//     if (!ifs.is_open()) {
//         std::cerr << "failed file open: " << filename << std::endl;
//         return;
//     }
//     std::string jsonContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
//     ifs.close();
//     if (document.Parse(jsonContent.c_str()).HasParseError()) {
//         std::cerr << "failed json parsing.." << std::endl;
//         return;
//     }
//     std::cout << "json parsing suceess" << std::endl;
// }

// void Rapid_Json_Handler::save_to_file(const std::string& filename) const {
//     StringBuffer buffer;
//     PrettyWriter<StringBuffer> writer(buffer);
//     document.Accept(writer);

//     std::ofstream ofs(filename);
//     if (ofs.is_open()) {
//         ofs << buffer.GetString();
//         ofs.close();
//         std::cout << "create and save json : " << filename << std::endl;
//     } else {
//         std::cerr << "failed file open :  " << filename << std::endl;
//     }
// }



