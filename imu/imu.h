#ifndef IMU_H
#define IMU_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "uart.h"
#define MAX_READ_SIZE   256

void send_command(const char* cmd);
void switch_sensor_mode(int mode);
void read_response_multiple(int count, char* read_data);
int imu_init(int interval_ms);
#endif

