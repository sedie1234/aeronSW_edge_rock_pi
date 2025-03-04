// Verbose Client TCP
// 포트를 왕창 열어서 TCP 연결을 유지하고, 각 쓰레드마다 메세지도 엄청 보내버려서 서버에 큰 부담을 주는 클라이언트
// 프로그램 실행 시, connect_count 만큼 서버의 TCP 연결을 시도. port 가 65535개? 있으니까 그만큼 열면 서버의 모든 포트를 점유.
// 하나의 연결당 Thread Count 만큼 전송하는 프로그램을 동작하여 메세지를 계속 보냄

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

#include "../utils/output.h"

using namespace std;

#define DESTINATION_IP "127.0.0.1"  //SERVER IP
#define DESTINATION_PORT 3939       //SERVER PORT
#define BUFFER_SIZE 1024

#define SERVER_CONNECT_NUM 1
#define VERBOSE_THREAD_NUM 1

// 연결되는 서버 정보를 저장하는 구조체
struct Server_Info{
    int server_socket;
    sockaddr_in server_Addr;
    //서버로 부터 받은 메세지 (있다면)
    char* message;              
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
vector<Server_Info*> server_Conn_Pool;
vector<pthread_t> read_thread_Manager; 
vector<pthread_t> write_thread_Manager; 

void* thread_handler(void* data);
void* server_connect(vector<Server_Info*>* conn_pool_ptr, string dest_ip, int dest_port, size_t connect_count);
void* gen_verbose_threads(vector<Server_Info*>* conn_pool_ptr, vector<pthread_t>* read_thread_Manager, vector<pthread_t>* write_thread_Manager, size_t thr_count);
void* send_message(void* server_data);
void* recv_message(void* recv_data);


int main(){

    pthread_t thread_controler;

    if(pthread_create(&thread_controler, nullptr, thread_handler, nullptr) != 0){
        cerr << "failed generate Thread " << endl; 
        return -1;    
    }

    char scf[50] = {0};
    
    while(true){
        memset(scf, 0, 50);
        cin >> scf;
        if((string)scf == "exit"){
            cout << "terminate server" << endl;
            pthread_detach(thread_controler); // Thread 분리 
            break;
        }else{
            //TODO 
        }
        
    }

    //종료 코드
    pthread_join(thread_controler, nullptr); // 메인 쓰레드 종료까지 대기
    // 메모리 할당 해제
    for (Server_Info * s : server_Conn_Pool){
        cout << "deallocate connection" << endl;
        delete s;
    }
    // 나머지 Thread 분리
    for (pthread_t hThread : read_thread_Manager){
        cout << "deallocate read_thread" << hThread << endl;
        pthread_detach(hThread);
    }
    for (pthread_t hThread : write_thread_Manager){
        cout << "deallocate write_thread" << hThread << endl;
        pthread_detach(hThread);
    }
    pthread_mutex_destroy(&mutex);

    return 0;
}

void* thread_handler(void* data){
    (void)data;
    // server_connect(dest_ip, dest_port, connect_count)
    // connect_count 개수만큼 TCP 포트를 여는 Client. Server Connection Pool 에 Connect 된 객체 저장됨.
    server_connect(&server_Conn_Pool, DESTINATION_IP, DESTINATION_PORT, SERVER_CONNECT_NUM);
    cout << SERVER_CONNECT_NUM << " Connection Successful" << server_Conn_Pool << endl;
    gen_verbose_threads(&server_Conn_Pool, &read_thread_Manager, &write_thread_Manager, VERBOSE_THREAD_NUM);

    // Join read threads
    for (pthread_t r_thread : read_thread_Manager) {
        pthread_detach(r_thread); // Thread 분리 실행
    }

    // Join write threads
    for (pthread_t w_thread : write_thread_Manager) {
        pthread_detach(w_thread);
    }
    while(true);
    return nullptr;
}


void* server_connect(vector<Server_Info*>* conn_pool_ptr, string dest_ip, int dest_port, size_t connect_count){

    for(size_t i=0; i < connect_count; i++){
        //소켓 생성 (TCP 연결)
        int sock = socket(AF_INET, SOCK_STREAM, 0);

        if(sock == -1){
            cerr << "failed generate client socket " << endl;
            perror("socket"); //meaning of the value of errno 
            exit(1);
        }

        //전송할 서버 정보 초기화
        Server_Info* server_Data = new Server_Info;
        memset(&server_Data->message, 0, sizeof(server_Data->message));
        memset(&server_Data->server_Addr, 0, sizeof(server_Data->server_Addr));
        server_Data->server_Addr.sin_family = AF_INET;
        server_Data->server_Addr.sin_addr.s_addr = inet_addr(dest_ip.c_str());
        server_Data->server_Addr.sin_port = htons(dest_port);

        //server socket 과 연결.
        if(connect(sock, (struct sockaddr*)&server_Data->server_Addr, sizeof(server_Data->server_Addr)) == -1) 
            perror("connect error");

        server_Data->server_socket = sock;
        conn_pool_ptr->push_back(server_Data);
    }
    return nullptr;
}


void* gen_verbose_threads(vector<Server_Info*>* conn_pool_ptr, vector<pthread_t>* read_thread_Manager, vector<pthread_t>* write_thread_Manager, size_t thr_count){

    for (Server_Info* server_Data : *conn_pool_ptr){
        
        // server socket read Thread 생성
        pthread_t* read_Thread = new pthread_t;

        cout << "r_thread" << read_Thread << endl;
        if (pthread_create(read_Thread, nullptr, recv_message, (void*)server_Data) != 0){
            cerr << "error creating read thread" << endl;
            perror("r_thread err");
        }else{
            read_thread_Manager->push_back(*read_Thread);
        }

        // server socket write Thread(connection x thr_count) 생성
        for (size_t i = 0; i < thr_count; i++){
            pthread_t* write_Thread = new pthread_t;
            cout << "wr_thread" << write_Thread << endl;
            if (pthread_create(write_Thread, nullptr, send_message, (void*)server_Data)){
                cerr << "error creating write thread" << endl;
                perror("w_thread err");
            }else{
                write_thread_Manager->push_back(*write_Thread);
            }
        }
    }

    return nullptr;
}

void* send_message(void* server_data){
    int cl_sock = static_cast<Server_Info*>(server_data)->server_socket;
    while(true){
        pthread_mutex_lock(&mutex);
        string message = to_string(1);
        write(cl_sock, message.c_str(), message.size()); //write 에선 char void* 타입의 버퍼가 인자값으로 들어가야함 따라서 c_str C스타일의 문자열(const char*)로 변환해주어야함.  
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return nullptr;
}

void* recv_message(void* recv_data){

    Server_Info* data = static_cast<Server_Info*>(recv_data);
    char* buffer = data->message;
    int recv_size;
    while(true){
        //int recv_size = read((data->server_socket), msg, sizeof(char)*BUFFER_SIZE);
        recv_size = recv(data->server_socket, buffer, sizeof(buffer), 0);       
        cout << recv_size << endl;
        if(recv_size > 0){
            cout << "recive from sever : " << buffer << endl;   
        }
    }

    return nullptr;
}
    
