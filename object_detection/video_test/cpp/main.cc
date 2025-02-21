// Copyright (c) 2023 by Rockchip Electronics Co., Ltd. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*-------------------------------------------
                Includes
-------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/*-------------------------------------------
                  Main Function
-------------------------------------------*/
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("%s <model_path> <CAM_index>\n", argv[0]);
        return -1;
    }

    const char *model_path = argv[1];
    int cam_index = atoi(argv[2]);

    cv::VideoCapture cap;

    TIMER infer_timer, real_timer, show_timer;

    int ret;
    rknn_app_context_t rknn_app_ctx;
    memset(&rknn_app_ctx, 0, sizeof(rknn_app_context_t));

    init_post_process();

    ret = init_yolov8_model(model_path, &rknn_app_ctx);

    if (ret != 0)
    {
        printf("init_yolov8_model fail! ret=%d model_path=%s\n", ret, model_path);
        goto out;
    }

    cap.open(cam_index,cv::CAP_V4L2);

    if(!cap.isOpened()){
        std::cerr << "[Error] : Could not open video source" << std::endl;
        return -1;
    }

    image_buffer_t src_image;
    memset(&src_image, 0, sizeof(image_buffer_t));
    
    while (true) {

        system("clear");

        real_timer.tik();
        show_timer.tik();

        cv::Mat frame;
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "[Error]: Video stream ended" << std::endl;
            break;
        }

        // src_image 설정
        src_image.width = frame.cols;
        src_image.height = frame.rows;
        src_image.virt_addr = frame.data;
        src_image.size = frame.total() * frame.elemSize();
        src_image.width_stride = 1;
        src_image.height_stride = 1;
        src_image.format = IMAGE_FORMAT_RGB888;

        object_detect_result_list od_results;
        
        infer_timer.tik();

        // YOLO 모델 추론
        ret = inference_yolov8_model(&rknn_app_ctx, &src_image, &od_results);
        infer_timer.tok();

        real_timer.tok();

        if (ret != 0) {
            printf("init_yolov8_model fail! ret=%d\n", ret);
            break;
        }

        char text[256];
        for (int i = 0; i < od_results.count; i++) {
            object_detect_result *det_result = &(od_results.results[i]);
            printf("%s @ (%d %d %d %d) %.3f\n", coco_cls_to_name(det_result->cls_id),
                det_result->box.left, det_result->box.top,
                det_result->box.right, det_result->box.bottom,
                det_result->prop);
            
            int x1 = det_result->box.left;
            int y1 = det_result->box.top;
            int x2 = det_result->box.right;
            int y2 = det_result->box.bottom;

            // OpenCV를 사용하여 직접 프레임에 사각형 그리기
            cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0), 2);

            // 텍스트 오버레이
            sprintf(text, "%s %.1f%%", coco_cls_to_name(det_result->cls_id), det_result->prop * 100);
            cv::putText(frame, text, cv::Point(x1, y1 - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
        }

        // OpenCV 화면 출력
        cv::imshow("inference", frame);
        
        show_timer.tok();

        printf("show ver frame fps : %f fps\n", 1000/show_timer.get_time());
        printf("inference time frame fps : %f fps\n", 1000/infer_timer.get_time());
        printf("real time frame fps : %f fps\n", 1000/real_timer.get_time());

        // ESC(27) 키를 누르면 종료
        if (cv::waitKey(1) == 27) {
            break;
        }
    }


    cap.release();
    cv::destroyAllWindows();



out:
    deinit_post_process();

    ret = release_yolov8_model(&rknn_app_ctx);
    if (ret != 0)
    {
        printf("release_yolov8_model fail! ret=%d\n", ret);
    }

    if (src_image.virt_addr != NULL)
    {
#if defined(RV1106_1103) 
        dma_buf_free(rknn_app_ctx.img_dma_buf.size, &rknn_app_ctx.img_dma_buf.dma_buf_fd, 
                rknn_app_ctx.img_dma_buf.dma_buf_virt_addr);
#else
        free(src_image.virt_addr);
#endif
    }
    return 0;
}
