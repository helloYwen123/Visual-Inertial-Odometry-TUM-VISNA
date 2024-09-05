# Visual-Inertial Tracking using Preintegrated Factors

This repo for project is built upon part of the code in practical course "Vision-based Navigation" (IN2106 24SS) taught at the Technical University of Munich(TUM). 
Final report: [report](Report_Visual_Inertial_odometry.pdf)

**Team Member**:


Pei-Ran Huang

Wenjie Xie

## Setup
```
git clone --recursive https://gitlab.lrz.de/24ss_visnav_vio/visual_inertial_slam.git
```
Run the sh file to install all- Prerequisites packages

```
cd ${yourworkspacename}
./install_dependencies.sh
./build_submodules.sh
```
After running the command, then need to build code
```
mkdir build && cd build
cmake ..
make
```
## Running 

Please adjust the variable names for the dataset and the status of IMU utilization as required.
```
cd ${yourworkspacename}
./build/odometry --dataset-path /data/euro_data/${datafolder}/mav0 --cam-calib euroc_ds_calib_visnav_type.json --use-imu true
```

