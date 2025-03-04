cmake_minimum_required(VERSION 3.10)
project(Subproject)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_C_STANDARD 11)

# 컴파일러 설정
set(CMAKE_CXX_COMPILER g++)

# 컴파일러 옵션 추가 (-Wall, -Wextra, -Wimplicit-fallthrough=5)
add_compile_options(-Wall -Wextra -Wimplicit-fallthrough=5)

# 프로젝트 디렉토리 설정
set(SRC_DIR src)
set(INC_DIR include)
set(BUILD_DIR build)
set(BIN_DIR bin)

# 실행 파일 이름 설정
set(TARGET_NAME run)

# 소스 파일 찾기
file(GLOB SRC_FILES "${SRC_DIR}/*.cpp")

# 실행 파일 생성
add_executable(${TARGET_NAME} ${SRC_FILES})

# 헤더 파일 포함 경로 추가
include_directories(${INC_DIR})

add_dependencies(${TARGET_NAME} imu_lib) ##imu_lib가 먼저 빌드되도록
# 외부 라이브러리 링크 설정
target_link_libraries(${TARGET_NAME}
    PRIVATE
    pthread
    rdkafka++
    rdkafka
    imu_lib
    cam_lib
)

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/imu)

# execute_process(COMMAND export LD_LIBRARY_PATH=$(pwd)/third_party/librdkafka)

# 라이브러리 및 런타임 경로 설정
set_target_properties(${TARGET_NAME} PROPERTIES
    # LINK_FLAGS "-Wl,-rpath,${CMAKE_SOURCE_DIR}/third_party/librdkafka"
    LINK_FLAGS "-Wl,-rpath,${CMAKE_CURRENT_SOURCE_DIR}/third_party/librdkafka"
)

target_link_directories(${TARGET_NAME} PRIVATE
    # ${CMAKE_SOURCE_DIR}/third_party/librdkafka
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/librdkafka
)

# 실행 후 실행 스크립트(run.sh) 실행 가능하도록 설정
add_custom_target(run2
    COMMAND ./run.sh
    DEPENDS ${TARGET_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(debug
    COMMAND ./debug.sh
    DEPENDS ${TARGET_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# # Clean 타겟 추가
# add_custom_target(clean
#     COMMAND ${CMAKE_COMMAND} -E rm -rf ${BUILD_DIR} ${BIN_DIR}
#     COMMENT "Cleaned build files."
# )

