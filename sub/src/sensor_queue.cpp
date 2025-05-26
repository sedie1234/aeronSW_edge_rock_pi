// #include "sensor_queue.h"

#include <queue>
#include <mutex>

// //센서 데이터 큐
// template<typename T>
// class DataQueue{
//     public :
//         void push(const T& item);
//         bool pop(T& item);
//     private:
//         std::queue<T> q_;
//         std::mutex mtx_;
// };

// void DataQueue<T>::push(const T& item){
//     std::lock_guard<std::mutex> lock(mtx_);
//     q_.push(item);
// }

// bool DataQueue<T>::pop(T& item){
//     return true;
// }
