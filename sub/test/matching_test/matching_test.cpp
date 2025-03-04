#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <error.h>

#include <ctime>
#include <chrono>

using namespace std;
using namespace chrono;

class Ref_Time_Node{

    private:
        Ref_Time_Node* prev_Node;
        long curr_ref_time;
        vector<Sensor_Node*> matched_Sen_Node;

    public:
        Ref_Time_Node(){
            prev_Node = nullptr;
            curr_ref_time = NULL;
        }

        void get_curr_time(){ 
            auto _time = chrono::system_clock::now();
            auto _mil = chrono::duration_cast<chrono::milliseconds>(_time.time_since_epoch());
            printf("%f", _time);
            
        }








};

struct Sensor_Node{
    char* data;
    Ref_Time_Node* ref_Node;
};

void* thread_handler(void* data);
void* gen_60fps_ref_time_node(void* data);  //16.67ms
void* gen_30fps_sensor_node(void* data); //33.33ms 
void* gen_15fps_sensor_node(void* data); //66.67ms 

int main(){


    pthread_t th_handler;
    if(pthread_create(&th_handler, nullptr, thread_handler, nullptr) != 0){
        cerr << "failed_gen_th_handler" << endl;
        return -1;
    }
    
    char scf[50] = {0};

    while(true){
        memset(scf, 0, 50);
        cin >> scf;
        if((string)scf == "exit"){
            cout << "terminate server" << endl;
            pthread_detach(th_handler); 
            break;
        }
    }
    pthread_join(th_handler, nullptr);
    return 0;
}

void* thread_handler(void* data){
    (void)data;
    pthread_t rf_t_nd_gen_thread;
    pthread_t s_30_nd_gen_thread;
    pthread_t s_15_nd_gen_thread;
    
    if(pthread_create(&rf_t_nd_gen_thread, nullptr, gen_60fps_ref_time_node, nullptr) != 0){
        cerr << "failed_ gen_60fps_ref_time_node" << endl;
        perror("gen_60fps_ref_time_node");
    }
    pthread_detach(rf_t_nd_gen_thread);

    if(pthread_create(&s_30_nd_gen_thread, nullptr, gen_30fps_sensor_node, nullptr) != 0){
        cerr << "failed_gen_30fps_sensor_node" << endl;
        perror("gen_30fps_sensor_node");
    }
    pthread_detach(s_30_nd_gen_thread);

    if(pthread_create(&s_15_nd_gen_thread, nullptr, gen_15fps_sensor_node, nullptr) != 0){
        cerr << "failed_gen_15fps_sensor_node" << endl;
        perror("gen_15fps_sensor_node");
    }
    pthread_detach(s_15_nd_gen_thread);

    return nullptr;

}

void* gen_60fps_ref_time_node(){

    auto time = chrono::system_clock::now();
    auto mil = chrono::duration_cast<chrono::milliseconds>(time.time_since_epoch());

    while(true){
        
        sleep(0.1667);
    }

}





