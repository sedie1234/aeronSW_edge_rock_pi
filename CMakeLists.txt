cmake_minimum_required(VERSION 3.10)
project(RockchipProject)

# # C++ 표준 설정
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)



###################################################################################################################
set(OBJECT_DETECTION_DIR "${CMAKE_SOURCE_DIR}/object_detection")
set(SUB_DIR "${CMAKE_SOURCE_DIR}/sub")

# message(STATUS "ROCKCHIPporject CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")

set(TARGET_SOC rk3588 CACHE STRING "Target SOC for object_detection module ")

# 🛠 object_detection 하위 CMakeLists.txt 실행
add_subdirectory(${CMAKE_SOURCE_DIR}/imu)
add_subdirectory(${OBJECT_DETECTION_DIR}/video_test/cpp)
add_subdirectory(${SUB_DIR})



