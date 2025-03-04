cmake_minimum_required(VERSION 3.10)
project(imuProject LANGUAGES C)

# C 표준 설정
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_STANDARD_REQUIRED ON)

#imu library 생성
add_library(imu_lib STATIC imu.c uart.c )
target_include_directories(imu_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})





