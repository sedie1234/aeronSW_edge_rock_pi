#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <csignal>
#include <iostream>

// run signal = true 
extern volatile sig_atomic_t run; // 여러곳에서 변경할 수 있기 때문에 volatile 로 선언
extern volatile sig_atomic_t req_sig; // Consumer 에서 main의 message require Signal 받았을때 1로 변경, 
extern std::string req_id;

void setup_sig_handler();
void sig_term(int sig);
void sig_req(int sig);

#endif