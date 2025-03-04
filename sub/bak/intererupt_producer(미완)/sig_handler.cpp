#include "sig_handler.h"

//extern 
//volatile sig_atomic_t run;
volatile std::atomic<bool> run;
volatile std::atomic<bool> req_sig;
volatile sigset_t sigset;
int sig_no;

// setup
void setup_sig_handler(sigset_t &sigset){
    run = true;
    req_sig = false;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);     //ctrl+c 
    sigaddset(&sigset, SIGTERM);    //kill 등 
    sigaddset(&sigset, SIGUSR1);    //req_topic 에서 req_message 받았을때

    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

    // std::signal(SIGINT, sig_term);   
    // std::signal(SIGTERM, sig_term); 
    // std::signal(SIGUSR1, sig_req); 
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