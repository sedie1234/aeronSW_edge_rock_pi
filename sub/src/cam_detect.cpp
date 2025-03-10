#include "cam_detect.h"

#include <thread>
#define CAM_VISIBLE 1
#define RECONNECT 5

Cam_Data cam_data;
int Cam_Detect::cam_connect(){
    for(int i=0; i<RECONNECT; i++){
        std::cout << i<<"번 재연결시도"<<std::endl;
        cam_data.cap.release();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));// 여유시간 얼마나?

        cam_data.cap.open(cam_data.cam_index,cv::CAP_V4L2); 

        //640*480 30fps
        std::cout << "Actual resolution: "
          << cam_data.cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x"
          << cam_data.cap.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;
          
        if(cam_data.cap.isOpened()){
            std::cerr << "Open video source" << std::endl;
            return 0;
        }
    }
    std::cerr << "[Error] : Could not open video source" << std::endl;
    return -1;
}
int Cam_Detect::init(char** argv)
{
#if 1
    const char *model_path = argv[1];
    cam_data.cam_index = atoi(argv[2]);
#elif 0
    const char *model_path = "../../object_detection/video_test/model/yolov8.rknn";
    int cam_index = 1;
#endif
      
    int ret;
    memset(&cam_data.rknn_app_ctx, 0, sizeof(rknn_app_context_t));

    init_post_process();
    
    ret = init_yolov8_model(model_path, &cam_data.rknn_app_ctx);

    if (ret != 0)
    {
        printf("init_yolov8_model fail! ret=%d model_path=%s\n", ret, model_path);
        goto out;
    }

    if(Cam_Detect::cam_connect()!=0){
        printf("cam_connect fail! \n");
        goto out;
    }

    memset(&cam_data.src_image, 0, sizeof(image_buffer_t));
    return 0;

out:

    deinit_post_process();

    ret = release_yolov8_model(&cam_data.rknn_app_ctx);
    if (ret != 0)
    {
        printf("release_yolov8_model fail! ret=%d\n", ret);
    }

    if (&cam_data.src_image.virt_addr != NULL)
    {
#if defined(RV1106_1103) 
        dma_buf_free(rknn_app_ctx.img_dma_buf.size, &rknn_app_ctx.img_dma_buf.dma_buf_fd, 
                    rknn_app_ctx.img_dma_buf.dma_buf_virt_addr);
#else
        free(&cam_data.src_image.virt_addr);
#endif
    }
    return -1;

}

object_detect_result_list Cam_Detect::object_detect(){ 

    TIMER infer_timer, real_timer, show_timer;
    int ret;

    // system("clear"); // 콘솔 화면 지우는 동작
    real_timer.tik();
    show_timer.tik();
    
    cv::Mat frame;
    // cam_data.cap >> frame;
    if (!cam_data.cap.read(frame)|| frame.empty()){ // 카메라 연결하는 부분
        std::cerr << "Error: Frame not captured!" << std::endl;
        goto out;
    }
    
    cam_data.src_image.width = frame.cols;
    cam_data.src_image.height = frame.rows;
    cam_data.src_image.virt_addr = frame.data;
    cam_data.src_image.size = frame.total() * frame.elemSize();
    cam_data.src_image.width_stride = 1;
    cam_data.src_image.height_stride = 1;
    cam_data.src_image.format = IMAGE_FORMAT_RGB888;

    object_detect_result_list od_results;

    infer_timer.tik();

    // YOLO 모델 추론
    ret = inference_yolov8_model(&cam_data.rknn_app_ctx, &cam_data.src_image, &od_results);//npu 도는 부분
    infer_timer.tok();

    real_timer.tok();

    if (ret != 0) {
        printf("init_yolov8_model fail! ret=%d\n", ret);
        goto out;
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
#if CAM_VISIBLE

        // OpenCV를 사용하여 직접 프레임에 사각형 그리기
        cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0), 2);

        // 텍스트 오버레이
        sprintf(text, "%s %.1f%%", coco_cls_to_name(det_result->cls_id), det_result->prop * 100);
        cv::putText(frame, text, cv::Point(x1, y1 - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
#endif
    }
#if CAM_VISIBLE
    // OpenCV 화면 출력
    cv::imshow("inference", frame);
#endif
    show_timer.tok();

    printf("show ver frame fps : %f fps\n", 1000/show_timer.get_time());
    printf("inference time frame fps : %f fps\n", 1000/infer_timer.get_time());
    printf("real time frame fps : %f fps\n", 1000/real_timer.get_time());

#if CAM_VISIBLE
    // ESC(27) 키를 누르면 종료
    if (cv::waitKey(1) == 27) {
        // break;
        printf("camera view exit\n");
        goto out;
    }
#endif
    return od_results;

out :
    // cam_data.cap.release();
    #if CAM_VISIBLE
    cv::destroyAllWindows();
    #endif
    cam_connect();

}


//YU 0401
// [ WARN:0@21702.680] global cap_v4l.cpp:999 open VIDEOIO(V4L2:/dev/video1): can't open camera by index
// [ WARN:0@21702.683] global cap.cpp:342 open VIDEOIO(V4L2): backend is generally available but can't be used to capture by index 
// = opencv에서 v4l2 백엔드 사용할 수 있지만 지정된 인덱스로 카메라를 열 수 없어 실패했다 
// = 연결은 끊겼는데 다시 연결했을때 이전의 사용프로세스가 종료되지 않은 상태?여서 다시 연결할 수 없는 상태인건가? => release 와 open 사이 time 추가
// 이 문구 후 sub(camera thread) 동작없음 
