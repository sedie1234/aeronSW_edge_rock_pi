#ifndef _SENSOR_QUEUE_H
#define _SENSOR_QUEUE_H

#include <queue>
#include <mutex>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

//센서 데이터 큐
template<typename T>
class DataQueue{
    public :
        
        void push(const T& item,const std::string& label){
            std::lock_guard<std::mutex> lock(mtx_);
            q_.push(item);
            
            if(q_.size()>=2) {
                std::cout << label << " ----------------------push size : "<<q_.size()<< std::endl;
            }
        }
        bool pop(T& item,const std::string& label){
            std::lock_guard<std::mutex> lock(mtx_);

            if(q_.empty()) return false;

            item=q_.front();
            q_.pop();

            return true;
        }

    private:
        std::queue<T> q_;
        std::mutex mtx_;
};



#endif