# Object Detection Solutions
- collection of object-detection solutions
- based on yolov8 model

## Contents
1. image_object_detection with cpu in python : coco dataset
2. image_object_detection with npu in python : coco dataset
3. image_object_detection with npu  in cpp : coco dataset
4. video_object_detection with cpu in python : coco dataset
5. video_object_detection with npu in python : coco dataset
6. video_object_detection with npu in cpp : coco dataset
7. image_object_detection with npu in python : custom dataset

## Requirements
- rockchip npu runtime install
```
$ sudo cp object_detection/rockchip_npu_runtime/librknnrt /usr/lib64/
```

## Image object detection
1. model download : yolov8n onnx model for rockchip sdk
```
$ cd image_infer_time_check/model/
$ sh download_model.sh
```

2. convert onnx model to rknn model
- COCO가 아닌 custom dataset을 활용하는 경우, calibration용 데이터셋을 구축해야 할 것
```
$ cd image_infer_time_check/python/
$ python3 convert.py ../model/yolov8n.onnx rk3588
```

3. inference in cpu and python : coco dataset
```
$ cd image_infer_time_check/python
$ python3 yolov8.py --model_path ../model/yolov8n.onnx --img_save
```

4. inference in npu and python : coco dataset
```
$ cd image_infer_time_check/python
$ python3 yolov8.py --model_path ../model/yolov8.rknn --target rk3588 --img_save
```

5. inference in npu and cpp : coco dataset
```
$ cd image_infer_time_check/cpp
$ mkdir build && cd build
$ cmake .. -DTARGET_SOC=rk3588
$ make
$ ./rknn_yolov8_demo ../../model/yolov8.rknn ../../model/bus.jpg
```

## Camera object detection
1. model download : yolov8n onnx model for rockchip sdk
```
$ cd video_test/model/
$ sh download_model.sh
```

2. convert onnx model to rknn model
- COCO가 아닌 custom dataset을 활용하는 경우, calibration용 데이터셋을 구축해야 할 것
```
$ cd video_test/python/
$ python3 convert.py ../model/yolov8n.onnx rk3588
```

3. inference in cpu and python : coco dataset
```
$ cd video_test/python
$ python3 yolov8.py --model_path ../model/yolov8n.onnx --cam [camera device index]
```

4. inference in npu and python : coco dataset
```
$ cd video_test/python
$ python3 yolov8.py --model_path ../model/yolov8.rknn --cam [camera device index] --target rk3588
```

5. inference in npu and cpp : coco dataset
```
$ cd video_test/cpp
$ mkdir build && cd build
$ cmake .. -DTARGET_SOC=rk3588
$ make
$ ./rknn_yolov8_demo ../../model/yolov8.rknn [camera device index]
```