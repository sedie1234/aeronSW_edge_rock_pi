#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "uart.h"

#define MAX_READ_SIZE   256
#define CMD_DELAY       100000    // 100ms
#define READ_INTERVAL   100000    // 100ms
#define UART_PORT       "/dev/ttyUSB0"
#define BAUD_RATE       B115200

struct UartDevice dev = {
    .filename = UART_PORT,
    .rate = BAUD_RATE
};

int imu_init(int interval_ms){
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "sp=%d\r\n", interval_ms);
    if (uart_start(&dev, false)) {
        printf("Failed to open UART device.\n");
        return 1;
    }

    printf("IMU UART Communication Started\n");
    send_command(cmd);         // 전송 주기 설정

    // uart_stop(&dev);
    return 0;
}

void send_command(const char* cmd) {
    printf("Sending: %s", cmd);  // 줄바꿈 포함됨
    uart_writes(&dev, cmd);
    uart_writes(&dev, "\r\n");
    // usleep(CMD_DELAY);  // 명령 간 간격
}

void switch_sensor_mode(int mode) {
    char cmd[32];

    snprintf(cmd, sizeof(cmd), "ss=%d\r\n", mode);
    send_command(cmd);         // 데이터 전송 모드 설정
    // usleep(300000);            // 센서 적용 대기 (300ms)
}

void read_response_multiple(int count, char* read_data) {
    for (int i = 0; i < count; i++) {

        // char read_data[MAX_READ_SIZE] = {0};
        int len = uart_reads(&dev, read_data, MAX_READ_SIZE);

        if (len > 0) {
            // 데이터 토큰 개수 확인 (공백 기준)
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            // ✅ ms 단위 timestamp 계산
            long long timestamp_ms = (long long)(ts.tv_sec) * 1000 + (ts.tv_nsec / 1000000);

            int token_count = 0;
            for (int i = 0; i < len; i++) {
                if (read_data[i] == ' ') token_count++;
            }
            // 📌 데이터 타입 태그
            const char* tag = "[unknown]";
            if (token_count >= 11) tag = "[ss=15]";
            else if (token_count >= 3) tag = "[ss=16]";

            // ✅ ms 단위 timestamp와 함께 출력
            printf("%s [%lld ms] %s\n", tag, timestamp_ms, read_data);
        }
    }
}

