#include <iostream>
#include <thread>
#include <vector>
#include <cstring> 
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

using namespace std;

#define SERVER_IP "127.0.0.1"
#define PACKET_SIZE 1024
#define SERVER_PORT 3939

// Client 정보를 담는 구조체
struct Client_Info{
    int socket;
    sockaddr_in client_Arddress; 
};

// Client 정보 구조체를 관리할 Vector. 
vector<Client_Info*> client_Pool; 
vector<pthread_t> client_Threads;

int server_socket;// 서버 소켓 (리눅스에선 소켓이 정수로 표현됨)
void* handle_client(void* data); //클라이언트 관리 Thread 함수
void* client_accept(void* data); //클라이언트 연결 Thread 함수

int main(int argc, char* argv[]){
    int optvalue = 1; 
    struct sockaddr_in server_Addr;
    //TCP Socket 생성,  
    server_socket = socket(AF_INET ,SOCK_STREAM, 0);

    //소켓 생성 실패시
    if(server_socket == -1){
        cerr << "failed generate server socket " << endl;
        perror("socket"); //meaning of the value of errno 
        return -1;
    }
    //소켓 옵션 설정. https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-setsockopt
    //LEVEL : SOL_SOCKET, SOL_NETILINLK, SOL_RAW, SOL_PACKET
    //S0_REUSEADDR : 주소 재사용 가능
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue) );

    memset(&server_Addr, 0, sizeof(server_Addr));

    //server_addr 멤버 설정
    cout << "set server addr ... " << endl;
    server_Addr.sin_family = AF_INET; //Address Family Internet
    server_Addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY = 0x0000_0000 어떤 IP 주소에서 들어오는지 다 받음 
    server_Addr.sin_port = htons(SERVER_PORT); // host bytes to network bytes short(16bit)... serverPort를 Little Endian(ex, 0x1234) 에서 Big Endian(ex, 0x3412) 방식으로 변경. (네트워크 표준은 Big Endian 이라 x86 OS에선 변환 필요. host가 bigendian 이여도 그냥 하면 된다는듯? 어차피 동작안한다고 함.)

    // 서버 소켓을 주소와 바인딩.
    cout << "binding ... " << endl;
    int bind_result = bind(server_socket, (struct sockaddr*)&server_Addr, sizeof(server_Addr));
    
    //바인딩 실패시
    if(bind_result < 0){
        cerr << "failed Binding" <<endl;
        perror("bind");
        close(server_socket);
        return -1;
    }

    //서버 클라이언트 요청 대기...
    listen(server_socket, 5); // 백로그 크기 : 5(동시에 처리 가능한 연결 요청 최대 개수)
    cout << "server " << SERVER_PORT << " port listening" << endl;

    //client_accept Thread 생성. 실패 시 retunr -1
    pthread_t thread_listen; 
    if(pthread_create(&thread_listen, nullptr, client_accept, nullptr) != 0){
        cerr << "failed generate Thread " << endl; 
        return -1;
    }

    char msg[PACKET_SIZE] = {0};
    
    while(true){
        memset(msg, 0, PACKET_SIZE);
        cin >> msg;
        if((string)msg == "li" or (string)msg=="list"){
            cout << "Connected Client List" << endl;
            for(Client_Info* c : client_Pool){
                cout << inet_ntoa(c->client_Arddress.sin_addr) << ":" << ntohs(c->client_Arddress.sin_port) <<endl;
            }

        }else if((string)msg == "exit"){
            cout << "terminate server" << endl;
            pthread_detach(thread_listen);
            break;

        }else{
            //TODO 
        }
        
    }

    // 종료 코드
    // Pool 메모리 할당 해제 
    pthread_join(thread_listen, nullptr);
    for (Client_Info* c : client_Pool){ // for c in client_pool
        cout << "Deallocate Client Pool memory : " << c << endl;
        delete c;
    }
    // Thread 메모리 할당 해제
    for (pthread_t hThread : client_Threads){
        cout << "Deallocate Client Thread memory : " << hThread << endl;
        pthread_detach(hThread);
    }
    return 0;
}

// 클라이언트 연결 Thread 함수
void* client_accept(void* data){
    while(1){
        int client_socket;
        struct sockaddr_in client_Addr;
        socklen_t client_addr_len = sizeof(client_Addr);

        //클라이언트 Connect 요청 시 클라이언트 소켓 생성.
        client_socket = accept(server_socket, (struct sockaddr*)&client_Addr, &client_addr_len);

        // 클라이언트 연결 실패시 소켓 닫고 다음 클라이언트 Connect 요청으로 넘어감. 
        if(client_socket == -1){
            cerr << "failed client connect accept" << endl;
            perror("client");
            close(client_socket);
            continue;
        }

        cout << "Client Connect Successful" << endl;
        cout << "Client Net_family : " << ntohs(client_Addr.sin_family) << endl;
        cout << "Client IP : " << inet_ntoa(client_Addr.sin_addr) << endl;
        cout << "Port : " << ntohs(client_Addr.sin_port) << endl;

        // 클라이언트의 데이터를 담은 구조체를 생성. 
        Client_Info* client_Data = new Client_Info;
        client_Data->socket = client_socket;
        client_Data->client_Arddress = client_Addr;

        // 연결된 클라이언트의 구조체 정보를 Vector 에 Push
        client_Pool.push_back(client_Data);

        // 클라이언트 처리 쓰레드 생성.
        pthread_t thread_client;
        pthread_create(&thread_client, nullptr, handle_client,(void*)client_Data);

        // 클라이언트 쓰레드 정보 Vector 에 Push
        client_Threads.push_back(thread_client);
    }
    return nullptr;
}

// 클라이언트에게 전송받은 데이터 처리 (연결되는 클라이언트 마다 쓰레드 단위로 수행 됨, TODO : Queue에 SubBoard 에서 모이는 데이터를 쌓아야함. Queue 는 임계영역이 될 것임으로 잘 처리 잘해야함.)
void* handle_client(void* data){
    Client_Info* client_Data = static_cast<Client_Info*>(data); //data인자값 Client_Info 형식으로 형변환하여 객체 생성
    int client_socket = client_Data->socket; 
    sockaddr_in client_Addr = client_Data->client_Arddress;

    char buffer[1024]; //PACKET_SIZE 1024
    int recv_size;

    do{
        memset(buffer, 0, PACKET_SIZE);
        recv_size = recv(client_socket, buffer, sizeof(buffer), 0); //recv(sock, buffer, n, flag) sock 으로부터 n byte 읽어서 buffer 에 저장. //flags : https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-recv 참고        

        // 클라이언트로 부터 데이터를 전송 받았을 경우. 
        if(recv_size > 0){
            cout << inet_ntoa(client_Addr.sin_addr) << "| " << ntohs(client_Addr.sin_port) << " : " << buffer << endl;
            send(client_socket, buffer, recv_size, 0); // 테스트용. 서버에서 받은 데이터 그대로 전송 
        }

    }while(recv_size > 0); //buffer 넘어서 더 있으면 반복. 
    
    // 전송이 끝나면 클라이언트 Pool 에서 삭제 하고, 소켓도 해제
    for (size_t i=0; i < client_Pool.size(); ++i){
        if(client_Pool[i]->socket == client_socket){
            // 클라이언트 정보 삭제
            cout << "disconnect Client : " << inet_ntoa(client_Pool[i]->client_Arddress.sin_addr) \
            << " : " << ntohs((client_Pool[i]->client_Arddress.sin_port)) << endl;
            send(client_socket, buffer, recv_size, 0);
            client_Pool.erase(client_Pool.begin() + i);
        }
    }
    delete client_Data;
    close(client_socket);
    return nullptr;
}