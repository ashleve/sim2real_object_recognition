# Sim2real Object Recognition
Scan real life objects with photogrammetry -> Generate synthetic dataset in simulation using Unity Perception Tools -> train with tiny YOLO -> port to Arduino based vehicle with Nvidia Jetson

## Photogrammetry
Software used: Meshroom (https://github.com/alicevision/meshroom)
![](images/photogrammetry/real_vs_scanned.png)

## YOLO
Model used: tiny YOLOv3 (from https://github.com/ultralytics/yolov5)
![](images/mosaic_examples/train_batch2.jpg)

## Prediction examples
![](images/prediction_examples/IMG_20210215_583568.png)
![](images/prediction_examples/IMG_20210215_120018.jpg)
