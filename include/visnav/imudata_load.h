#pragma once

#include <Eigen/Core>
#include <sophus/se3.hpp>
#include <fstream>
#include <visnav/common_types.h>
#include <iostream>

namespace visnav {

// data structure for Gyroscope
struct GyroData {
  Timestamp timestamp_ns;
  Eigen::Vector3d data;
  GyroData(Timestamp t, const Eigen::Vector3d& d) : timestamp_ns(t), data(d) {}
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
// data structure for Accelerometer
struct AccelData {
  Timestamp timestamp_ns;
  Eigen::Vector3d data;
  AccelData(Timestamp t, const Eigen::Vector3d& d) : timestamp_ns(t), data(d) {}
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

// Define the base ImuDataset class
class ImuDataset {
 public:
  virtual const std::vector<AccelData>& get_accel_data() const = 0;
  virtual const std::vector<GyroData>& get_gyro_data() const = 0;
  virtual const std::vector<int64_t>& get_gt_timestamps() const = 0;
  virtual const std::vector<int64_t>& get_gt_pose_timestamps() const = 0;
  virtual const std::vector<Sophus::SE3d,
                            Eigen::aligned_allocator<Sophus::SE3d>>&
  get_gt_state_data() const = 0;
  virtual const std::vector<Eigen::Vector3d>&
  get_gt_pose_data() const = 0;
  virtual int64_t get_mocap_to_imu_offset_ns() const = 0;

  virtual ~ImuDataset(){};
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class EurocImuDataset : public ImuDataset {
  std::string path;
  std::vector<AccelData> accel_data;
  std::vector<GyroData> gyro_data;
  std::vector<int64_t> gt_timestamps; // true timestamps
  std::vector<int64_t> gt_pose_timestamps;  
   std::vector<Sophus::SE3d, Eigen::aligned_allocator<Sophus::SE3d>>
       gt_state_data; //ground true pose data
    std::vector<Eigen::Vector3d> gt_pose_data; //ground true pose data
  int64_t mocap_to_imu_offset_ns = 0;  // capture time offset between motion and imu
 
 public:
  ~EurocImuDataset(){};

  const std::vector<AccelData>& get_accel_data() const { return accel_data; }
  const std::vector<GyroData>& get_gyro_data() const { return gyro_data; }
  const std::vector<int64_t>& get_gt_timestamps() const {
    return gt_timestamps;
  }
  const std::vector<int64_t>& get_gt_pose_timestamps() const{
    return gt_pose_timestamps;
  }
  const std::vector<Sophus::SE3d, Eigen::aligned_allocator<Sophus::SE3d>>&
  get_gt_state_data() const {
    return gt_state_data;
  }
  const std::vector<Eigen::Vector3d>&
  get_gt_pose_data() const {
    return gt_pose_data;
  };

  int64_t get_mocap_to_imu_offset_ns() const { return mocap_to_imu_offset_ns; }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  friend class EurocIO;
};

typedef std::shared_ptr<ImuDataset> ImuDatasetPtr; // base class(ImuDataset : EurocImuDataset) pointer

// Define the base class for IO interface for the ImuDataset
class DatasetIoInterface {
 public:
  virtual void read(const std::string& path) = 0;
  virtual void reset() = 0;
  virtual ImuDatasetPtr get_data() = 0;

  virtual ~DatasetIoInterface(){};
};

// Define the derived class for IO interface for the Euroc ImuDataset
class EurocIO : public DatasetIoInterface {
public:
    EurocIO() {}

    void read(const std::string& path) {

        std::ifstream os(path , std::ios::binary);
        if (!os.is_open()) {
        std::cerr << "No dataset found in " << path << std::endl;
        } else {
        data.reset(new EurocImuDataset); //?

        data->path = path;

        read_imu_data(path + "/imu0/");

        std::ifstream gt_states(path + "/state_groundtruth_estimate0/data.csv",
                                std::ios::binary);
        std::ifstream gt_poses(path + "/leica0/data.csv", std::ios::binary);
        if (gt_states.is_open()) {
            read_gt_data_state(path + "/state_groundtruth_estimate0/");
        } else if (gt_poses.is_open()) {
            read_gt_data_pose(path + "/leica0/");
        }
        }
    }

    void reset() { data.reset(); } //?
    ImuDatasetPtr get_data() {return data;}  //return pointer to base class(but actually points the base child class)

private:
    std::shared_ptr<EurocImuDataset> data;

    void read_imu_data(const std::string& path) {
        data->accel_data.clear();
        data->gyro_data.clear();

        std::ifstream imu_data(path + "data.csv");
        std::string line;
        while (std::getline(imu_data, line)) {
        if (line[0] == '#') continue;

        std::vector<double> Data(7);
        std::string item;
        size_t pos = 0;
        int count = 0;

        while ((pos = line.find(',')) != std::string::npos && count < 6) {
            item = line.substr(0, pos);
            Data[count++] = std::stod(item);
            line.erase(0, pos + 1);
        }
        Data[6] = std::stod(line);

        uint64_t timestamp = static_cast<uint64_t>(Data[0]);
        Eigen::Vector3d gyro(Data[1], Data[2], Data[3]);
        Eigen::Vector3d accel(Data[4], Data[5], Data[6]);

        data->accel_data.emplace_back(timestamp, accel);
        data->gyro_data.emplace_back(timestamp, gyro);
        }
    }

    void read_gt_data_state(const std::string& path) {
        data->gt_timestamps.clear();
        data->gt_state_data.clear();

        std::ifstream gt_state_Data(path + "data.csv");
        std::string line;
        while (std::getline(gt_state_Data, line)) {
        if (line[0] == '#') continue;

        std::vector<double> Data(17);
        std::string item;
        size_t p = 0;
        int count = 0;

        while ((p = line.find(',')) != std::string::npos && count < 16) {
            item = line.substr(0, p);
            Data[count++] = std::stod(item);
            line.erase(0, p + 1);
        }
        Data[16] = std::stod(line);

        uint64_t timestamp = static_cast<uint64_t>(Data[0]);
        Eigen::Vector3d pos(Data[1], Data[2], Data[3]);
        Eigen::Quaterniond q(Data[4],Data[5], Data[6], Data[7]); //  qw qx qy qz
        Eigen::Vector3d vel(Data[8], Data[9], Data[10]);
        Eigen::Vector3d accel_bias(Data[11], Data[12], Data[13]);
        Eigen::Vector3d gyro_bias(Data[14], Data[15], Data[16]);

        data->gt_timestamps.emplace_back(timestamp);
        data->gt_state_data.emplace_back(q, pos);
        }
    }

    void read_gt_data_pose(const std::string& path) {
        data->gt_pose_timestamps.clear();
        data->gt_pose_data.clear();

        std::ifstream gt_pose_Data(path + "data.csv");
        std::string line;
        while (std::getline(gt_pose_Data, line)) {
        if (line[0] == '#') continue;

        std::stringstream ss(line);

        char tmp;
        uint64_t timestamp;
        Eigen::Vector3d pos;

        ss >> timestamp >> tmp >> pos[0] >> tmp >> pos[1] >> tmp >> pos[2];
  

        data->gt_pose_timestamps.emplace_back(timestamp);
        data->gt_pose_data.emplace_back(pos);
        }
    }
  
};

typedef std::shared_ptr<DatasetIoInterface> DatasetIoInterfacePtr;

// To be compatible with multiple types of dataset,
class DatasetIoFactory {
 public:
  static DatasetIoInterfacePtr getDatasetIo(const std::string& dataset_type) {
    if (dataset_type == "euroc") {
      return DatasetIoInterfacePtr(new EurocIO()); // base class type pointer is assigned with child class type pointer
    } else {
      std::cerr << "Dataset type " << dataset_type << " is not supported"
                << std::endl;
      std::abort();
    }
  }
};

void load_imu(
    DatasetIoInterfacePtr& dataset_io,
    std::queue<std::pair<Timestamp, ImuData<double>::Ptr>>& imu_data_queue, //  timestamp & data(timestamp acceleration gyro)
    std::vector<Timestamp>& timestamps_imu, 
    CalibAccelBias<double>& calib_accel,
    CalibGyroBias<double>& calib_gyro) {
  for (size_t i = 0; i < dataset_io->get_data()->get_accel_data().size(); i++) {
    ImuData<double>::Ptr data(new ImuData<double>);  // create imu data object
    data->t_ns = dataset_io->get_data()->get_gyro_data()[i].timestamp_ns;  // assign timestamp
    timestamps_imu.push_back(data->t_ns);
    data->accel = calib_accel.getCalibrated(
        dataset_io->get_data()->get_accel_data()[i].data);  // calibrate acceleration and gyrometer ; and then assign data-> accel & gyro with calibrated version
    data->gyro = calib_gyro.getCalibrated(
        dataset_io->get_data()->get_gyro_data()[i].data);
    imu_data_queue.push(std::make_pair(data->t_ns, data));
  }
}


}