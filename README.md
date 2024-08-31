# Visual-Inertial Tracking using Preintegrated Factors

This repo for project is built upon part of the code in practical course "Vision-based Navigation" (IN2106 24SS) taught at the Technical University of Munich(TUM). 

**Team Member**:


Pei-Ran Huang

Wenjie Xie

Command to run:

mkdir build


cd build

cmake ..

make

./build/odometry --dataset-path data/(datafolder)/mav0/ --cam-calib euroc_ds_calib.json --imu true
