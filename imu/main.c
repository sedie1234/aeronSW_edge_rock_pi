/*
 * main.c
 *
 * @date 2019/08/09
 * @author Cosmin Tanislav
 * @author Cristian Fatu
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "uart.h"

#define MAX_READ_SIZE   235
#define CMD_DELAY   100000

struct UartDevice dev = {
    .filename = "/dev/ttyUSB0",
    .rate = B115200
};

void send_command(const char* cmd);
void read_response();

int main(int argc, char* argv[]) {
    int rc;

    if(argc != 3){
        printf("Usage : %s <interval(ms)> <data>\n", argv[0]);
        return;
    }

    char cmd_interval[10] = "sp=";
    char cmd_data[10] = "ss=";

    strcat(cmd_interval, argv[1]);
    strcat(cmd_interval, "\n");

    strcat(cmd_data, argv[2]);
    strcat(cmd_data, "\n");

    rc = uart_start(&dev, false);
    if (rc) {
        printf("Failed to start UART\n");
        return rc;
    }

    printf("IMU UART Communication Started\n");

    send_command(cmd_interval);
    send_command(cmd_data);
 
    // send_command("sp=10");
    // send_command("ss=12");
    

    while (1) {
        read_response();
    }

    uart_stop(&dev);

    return 0;
}

void send_command(const char* cmd) {
    printf("Sending: %s\n", cmd);
    uart_writes(&dev, cmd);
    uart_writes(&dev, "\r\n");
    usleep(CMD_DELAY);
}

void read_response() {
    char read_data[MAX_READ_SIZE];
    int len = uart_reads(&dev, read_data, MAX_READ_SIZE);
    if(len > 0) {
        printf("%s\n", read_data);
    }
}
