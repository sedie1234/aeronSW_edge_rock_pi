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
            
            
            std::cout << label << " ----------------------push size : "<<q_.size()<< std::endl;
            
        }
        bool pop(T& item,const std::string& label){
            std::lock_guard<std::mutex> lock(mtx_);

            std::cout <<"[data_queue]----------------------pop size : "<<q_.size()<< std::endl;
            if(q_.empty()) return false;
            
            // 큐의 모든 데이터 pop , 가장 마지막 데이터만 리턴
            while(!q_.empty()) {
                item=q_.front();
                q_.pop();            
            }

            return true;
        }

    private:
        std::queue<T> q_;
        std::mutex mtx_;
};



#endif