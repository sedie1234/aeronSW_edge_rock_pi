#ifndef _CAM_DETECT_H
#define _CAM_DETECT_H


//카메라 관련
#include "yolov8.h"
#include "image_utils.h"
#include "file_utils.h"
#include "image_drawing.h"
#include "easy_timer.h"

#include <sys/time.h>

#include <opencv2/opencv.hpp>
#include <iostream>

#if defined(RV1106_1103) 
    #include "dma_alloc.hpp"
#endif


//카메라 이미지 인식 관련 데이터
struct Cam_Data {
    int cam_index;
    cv::VideoCapture cap;
    image_buffer_t src_image;
    rknn_app_context_t rknn_app_ctx;
};

class Cam_Detect{
    private : 

    public :// 멤버 함수
        static int init(char** argv);

        object_detect_result_list object_detect();
        static int cam_connect();


};


#endif