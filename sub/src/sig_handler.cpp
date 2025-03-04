#include "sig_handler.h"

//extern 
volatile sig_atomic_t run;
volatile sig_atomic_t req_sig;
std::string req_id;

// setup
void setup_sig_handler(){
    run = 1;
    req_sig = 0;
    std::signal(SIGINT, sig_term);  //ctrl+c  
    std::signal(SIGTERM, sig_term); //kill 등 
    std::signal(SIGUSR1, sig_req); //req_topic 에서 req_message 받았을때
}

//종료 신호 수신
void sig_term(int sig){
    (void)sig;
    run = 0;
    std::cerr << "Terminate Program.." << std::endl;   
}

//req 신호 수신
void sig_req(int sig){
    (void)sig;
    req_sig = 1;
    std::cerr << "SIG_REQ :: recv_req message" << std::endl;
}