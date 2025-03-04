//inline 함수나 define macro, template 은 .inl 로 분리,
//struct type 은 따로 처리 안해놨으니 필요하면 호출해서 써야함,,

#include "rapid_json_handler.h"

template <typename T> 
void Rapid_Json_Handler::add_member_p(const std::string key, const T &value){
    if constexpr (std::is_same_v<T, std::string>){
        Pointer(key.c_str()).Set(document, value.c_str(), document.GetAllocator());
    } else {
        Pointer(key.c_str()).Set(document, value, document.GetAllocator());
    }
}
