# Visual-Inertial Tracking using Preintegrated Factors

This repo for project is built upon part of the code in practical course "Vision-based Navigation" (IN2106 24SS) taught at the Technical University of Munich(TUM). 
Final report: [report](Report_Visual_Inertial_odometry.pdf)

**Team Member**:


Pei-Ran Huang

Wenjie Xie

## Setup
```
git clone --recursive https://github.com/helloYwen123/Visual-Inertial-Odometry-TUM_Visnav.git
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
Note our performance testing for algorithm is only based on benchmark [EuRoC](https://projects.asl.ethz.ch/datasets/doku.php?id=kmavvisualinertialdatasets) from ETH
## Running 

Please adjust the variable names for the dataset and the status of IMU utilization as required.
```
cd ${yourworkspacename}
./build/odometry --dataset-path /data/euro_data/${datafolder}/mav0 --cam-calib euroc_ds_calib_visnav_type.json --use-imu true
```
### demo video
You can see our results presentation in the [demo](https://drive.google.com/file/d/1wb0d1FAokyZxkMmGQOABPveuwGAcV28H/view?usp=drive_link)

