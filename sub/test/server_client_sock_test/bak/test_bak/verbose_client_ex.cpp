//Client TCP

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <error.h>

using namespace std;

#define DESTINATION_IP "127.0.0.1"  //SERVER IP
#define DESTINATION_PORT 3939       //SERVER PORT
#define BUFFER_SIZE 1024

struct Recieve_Data{
    char* message;
    int* server_socket;
};

vector<pthread_t> send_client_Threads;
vector<pthread_t> recv_client_Threads;

void* send_message(void* sock);
void* recv_message(void* recv_data);

int main(int argc, char* argv[]){
    // 클라이언트 측 소켓 생성 (TCP 연결)
    int sock=socket(AF_INET, SOCK_STREAM, 0);

    //소켓 생성 실패시
    if(sock == -1){
        cerr << "failed generate socket " << endl;
        perror("socket"); //meaning of the value of errno 
        return -1;
    }

    // 테스트 용으로 argument 받아서 전송
    if(argc!=3){
        cout << "Usage : " << argv[0] << "<SERVER IP> <SERVER PORT>" <<endl;
        return -1;
    }

    if(sock==-1){
        perror("sock error");
    }

    //전송할 서버 정보 초기화
    struct sockaddr_in server_Addr; 
    memset(&server_Addr, 0, sizeof(server_Addr));
    server_Addr.sin_family = AF_INET;
    server_Addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_Addr.sin_port = htons(atoi(argv[2]));
    //server_Addr.sin_addr.s_addr = inet_addr(DESTINATION_IP);
    //server_Addr.sin_port = htons(DESTINATION_PORT);

    //서버 주소로 해당 클라이언트의 소켓으로 연결 요청. (실패시 error)
    if(connect(sock, (struct sockaddr*)&server_Addr, sizeof(server_Addr)) == -1) 
        perror("connect error");

    //메세지 계속 보내는 시끄러운 클라이언트 쓰레드 5개 생성 (나중에 각 센서별로 데이터 받아서 보내는걸로 변경, Port 도 각각 다르게 받아야함.)
    //서버가 메세지를 받을때마다 다시 그대로 돌려주게 하면 
    //한개의 클라이언트에서 데이터 여러개 보낼 예정.

    // verbose Client 생성
    // pthread_t pThread[2];
    // Recieve_Data recv_Data;

    // for(size_t i=0; i<5; i++){
    //     pthread_create(&pThread[0], nullptr, send_message,(void*)sock);

    // }
    // // 서버로 부터 메시지 수신 할 경우 받아서 출력
    // for(size_t i=0; i<5; i++){
    //     pthread_create(&pThread[1], nullptr, recv_message,(void*)&recv_Data);
    // }

    
    // for(size_t i=0; i<5; i++){

    // }


    close(sock);
}

void* send_message(void* sock){
    int* cs = (int*)sock;
    for(int i=0; i<100; i++){
        cout << "send --> " << i << endl;
        string message = to_string(i);
        write(*cs, message.c_str(), message.size()); //write 에선 char void* 타입의 버퍼가 인자값으로 들어가야함 따라서 c_str C스타일의 문자열(const char*)로 변환해주어야함.
        sleep(1);
    }
}

void* recv_message(void* recv_data){

    Recieve_Data* data = (Recieve_Data*)recv_data;
    char* msg = data->message;

    while(true){
        int str_len = read(*(data->server_socket), msg, sizeof(char)*BUFFER_SIZE);
        if(str_len != -1){
            cout << "recive from sever : " << msg << endl;   
        }
    }
}
    
