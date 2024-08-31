/**
BSD 3-Clause License

Copyright (c) 2018, Vladyslav Usenko and Nikolaus Demmel.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <fstream>
#include <thread>

#include <ceres/ceres.h>

#include <opengv/absolute_pose/CentralAbsoluteAdapter.hpp>
#include <opengv/absolute_pose/methods.hpp>
#include <opengv/relative_pose/CentralRelativeAdapter.hpp>
#include <opengv/sac/Ransac.hpp>
#include <opengv/sac_problems/absolute_pose/AbsolutePoseSacProblem.hpp>
#include <opengv/triangulation/methods.hpp>

#include <visnav/common_types.h>
#include <visnav/serialization.h>

#include <visnav/reprojection.h>
#include <visnav/local_parameterization_se3.hpp>

#include <visnav/tracks.h>

#include <visnav/preintegration_imu/imu_types.h>
#include <visnav/preintegration_imu/preintegration.h>
#include <visnav/preintegration_imu/calib_bias.hpp>
#include <visnav/preintegration_imu/utils/assert.h>
#include <visnav/preintegration_imu/utils/eigen_utils.hpp>

namespace visnav {

// save map with all features and matches
void save_map_file(const std::string& map_path, const Corners& feature_corners,
                   const Matches& feature_matches,
                   const FeatureTracks& feature_tracks,
                   const FeatureTracks& outlier_tracks, const Cameras& cameras,
                   const Landmarks& landmarks) {
  {
    std::ofstream os(map_path, std::ios::binary);

    if (os.is_open()) {
      cereal::BinaryOutputArchive archive(os);
      archive(feature_corners);
      archive(feature_matches);
      archive(feature_tracks);
      archive(outlier_tracks);
      archive(cameras);
      archive(landmarks);

      size_t num_obs = 0;
      for (const auto& kv : landmarks) {
        num_obs += kv.second.obs.size();
      }
      std::cout << "Saved map as " << map_path << " (" << cameras.size()
                << " cameras, " << landmarks.size() << " landmarks, " << num_obs
                << " observations)" << std::endl;
    } else {
      std::cout << "Failed to save map as " << map_path << std::endl;
    }
  }
}

// load map with all features and matches
void load_map_file(const std::string& map_path, Corners& feature_corners,
                   Matches& feature_matches, FeatureTracks& feature_tracks,
                   FeatureTracks& outlier_tracks, Cameras& cameras,
                   Landmarks& landmarks) {
  {
    std::ifstream is(map_path, std::ios::binary);

    if (is.is_open()) {
      cereal::BinaryInputArchive archive(is);
      archive(feature_corners);
      archive(feature_matches);
      archive(feature_tracks);
      archive(outlier_tracks);
      archive(cameras);
      archive(landmarks);

      size_t num_obs = 0;
      for (const auto& kv : landmarks) {
        num_obs += kv.second.obs.size();
      }
      std::cout << "Loaded map from " << map_path << " (" << cameras.size()
                << " cameras, " << landmarks.size() << " landmarks, " << num_obs
                << " observations)" << std::endl;
    } else {
      std::cout << "Failed to load map from " << map_path << std::endl;
    }
  }
}

// Create new landmarks from shared feature tracks if they don't already exist.
// The two cameras must be in the map already.
// Returns the number of newly created landmarks.
int add_new_landmarks_between_cams(const FrameCamId& fcid0,
                                   const FrameCamId& fcid1,
                                   const Calibration& calib_cam,
                                   const Corners& feature_corners,
                                   const FeatureTracks& feature_tracks,
                                   const Cameras& cameras,
                                   Landmarks& landmarks) {
  // shared_track_ids will contain all track ids shared between the two images,
  // including existing landmarks
  std::vector<TrackId> shared_track_ids;

  // find shared feature tracks
  const std::set<FrameCamId> fcids = {fcid0, fcid1};
  if (!GetTracksInImages(fcids, feature_tracks, shared_track_ids)) {
    return 0;
  }

  // at the end of the function this will contain all newly added track ids
  std::vector<TrackId> new_track_ids;

// TODO SHEET 4: Triangulate all new features and add to the map
  // UNUSED(calib_cam);
  // UNUSED(feature_corners);
  // UNUSED(cameras);
  // UNUSED(landmarks);
   // Transformations from cawmeras to world
  const auto& T_w_c0 = cameras.at(fcid0).T_w_c;
  const auto& T_w_c1 = cameras.at(fcid1).T_w_c;

  // extract the 2d-feature corners
  const auto& keypoints0 = feature_corners.at(fcid0).corners;
  const auto& keypoints1 = feature_corners.at(fcid1).corners;

  // relative transformation matrix
  const auto& T_0_1 = T_w_c0.inverse() * T_w_c1;



  for (const auto& track_id : shared_track_ids) {
 
    if (landmarks.find(track_id) != landmarks.end()) {
      continue;
    }

    // Get corresponding 3d-points
    const auto& feature_track = feature_tracks.at(track_id);
    Eigen::Vector2d pt0 = keypoints0[feature_track.at(fcid0)];
    Eigen::Vector2d pt1 = keypoints1[feature_track.at(fcid1)];
    auto cam0 = calib_cam.intrinsics[fcid0.cam_id]; //left camera and right camera
    auto cam1 = calib_cam.intrinsics[fcid1.cam_id]; 
    // OpenGV triangulate
    opengv::bearingVectors_t bearingVectors1;
    opengv::bearingVectors_t bearingVectors2;

    bearingVectors1.push_back(cam0->unproject(pt0));
    bearingVectors2.push_back(cam1->unproject(pt1));

    opengv::relative_pose::CentralRelativeAdapter adapter(
        bearingVectors1, bearingVectors2, T_0_1.translation(), T_0_1.rotationMatrix());

    Eigen::Vector3d triangulated_point = opengv::triangulation::triangulate(adapter, 0);

    // convert trangulated points to world frame 
    Eigen::Vector3d world_point = T_w_c0 * triangulated_point;

    // new landmark
    Landmark landmark;
    landmark.p = world_point;

    // fill the observed value for landmarks
    for (const auto& [fcid, feature_id] : feature_track) {
      if(cameras.count(fcid)>0){landmark.obs[fcid] = feature_id;}
    }

    // add new landmarks to map
    landmarks[track_id] = landmark;
    new_track_ids.push_back(track_id);
  }
  return new_track_ids.size();
}

// Initialize the scene from a stereo pair, using the known transformation from
// camera calibration. This adds the inital two cameras and triangulates shared
// landmarks.
// Note: in principle we could also initialize a map from another images pair
// using the transformation from the pairwise matching with the 5-point
// algorithm. However, using a stereo pair has the advantage that the map is
// initialized with metric scale.
bool initialize_scene_from_stereo_pair(const FrameCamId& fcid0,
                                       const FrameCamId& fcid1,
                                       const Calibration& calib_cam,
                                       const Corners& feature_corners,
                                       const FeatureTracks& feature_tracks,
                                       Cameras& cameras, Landmarks& landmarks) {
  // check that the two image ids refer to a stereo pair
  if (!(fcid0.frame_id == fcid1.frame_id && fcid0.cam_id != fcid1.cam_id)) {
    std::cerr << "Images " << fcid0 << " and " << fcid1
              << " don't form a stereo pair. Cannot initialize." << std::endl;
    return false;
  }

// TODO SHEET 4: Initialize scene (add initial cameras and landmarks)
  // UNUSED(calib_cam);
  // UNUSED(feature_corners);
  // UNUSED(feature_tracks);
  // UNUSED(cameras);
  // UNUSED(landmarks);
  Camera left_camera;
  left_camera.T_w_c = calib_cam.T_i_c[0];

  Camera right_camera;
  right_camera.T_w_c = calib_cam.T_i_c[1];

  cameras[fcid0] = left_camera;
  cameras[fcid1] = right_camera;

  add_new_landmarks_between_cams(fcid0, fcid1, calib_cam, feature_corners, feature_tracks, cameras, landmarks);


  return true;
}

// Localize a new camera in the map given a set of observed landmarks. We use
// pnp and ransac to localize the camera in the presence of outlier tracks.
// After finding an inlier set with pnp, we do non-linear refinement using all
// inliers and also update the set of inliers using the refined pose.
//
// shared_track_ids already contains those tracks which the new image shares
// with the landmarks (but some might be outliers).
//
// We return the refined pose and the set of track ids for all inliers.
//
// The inlier threshold is given in pixels. See also the opengv documentation on
// how to convert this to a ransac threshold:
// http://laurentkneip.github.io/opengv/page_how_to_use.html#sec_threshold
void localize_camera(
    const FrameCamId& fcid, const std::vector<TrackId>& shared_track_ids,
    const Calibration& calib_cam, const Corners& feature_corners,
    const FeatureTracks& feature_tracks, const Landmarks& landmarks,
    const double reprojection_error_pnp_inlier_threshold_pixel,
    Sophus::SE3d& T_w_c, std::vector<TrackId>& inlier_track_ids) {
  inlier_track_ids.clear();

// TODO SHEET 4: Localize a new image in a given map
  // UNUSED(fcid);
  // UNUSED(shared_track_ids);
  // UNUSED(calib_cam);
  // UNUSED(feature_corners);
  // UNUSED(feature_tracks);
  // UNUSED(landmarks);
  // UNUSED(T_w_c);
  // UNUSED(reprojection_error_pnp_inlier_threshold_pixel);
  std::vector<Eigen::Vector3d> points3d;
  std::vector<Eigen::Vector2d> points2d;
  for(const auto& track_id: shared_track_ids){
    if(landmarks.find(track_id) != landmarks.end()){
      const Landmark& landmark = landmarks.at(track_id);
      const FeatureTrack& feature_track = feature_tracks.at(track_id);
         if(feature_track.find(fcid)!=feature_track.end()){
          points3d.push_back(landmark.p);
          points2d.push_back(feature_corners.at(fcid).corners[feature_track.at(fcid)]);
         }
      }
  }
  auto cam = calib_cam.intrinsics[fcid.cam_id];
  opengv::bearingVectors_t bearingVectors;
  opengv::points_t points;
  for(size_t i=0; i<points2d.size(); i++){
      Eigen::Vector3d bearing(cam->unproject(points2d[i]));
      bearing.normalized(); //normalize ? Why ?
      bearingVectors.push_back(bearing);
      points.push_back(points3d[i]);
  }
  opengv::absolute_pose::CentralAbsoluteAdapter adapter(bearingVectors, points);

      typedef 
      opengv::sac_problems::absolute_pose::AbsolutePoseSacProblem 
      AbsolutePoseSacProblem;

      std::shared_ptr<AbsolutePoseSacProblem> absposeproblem_ptr (
        new AbsolutePoseSacProblem(adapter,AbsolutePoseSacProblem::KNEIP));

      opengv::sac::Ransac<AbsolutePoseSacProblem> ransac;

      ransac.sac_model_ = absposeproblem_ptr;
      ransac.threshold_ = 1.0- std::cos(std::atan(reprojection_error_pnp_inlier_threshold_pixel/ 500.0));
      ransac.max_iterations_ = 1000;

      if(!ransac.computeModel()){return;};
      
      adapter.sett(ransac.model_coefficients_.block<3,1>(0,3));
      adapter.setR(ransac.model_coefficients_.block<3,3>(0,0));

      opengv::transformation_t non_linear_transfomation = opengv::absolute_pose::optimize_nonlinear(adapter,ransac.inliers_);
      

      //re-evaluate the inliers
      std::vector<int> refined_inliers;
      absposeproblem_ptr->selectWithinDistance(non_linear_transfomation, ransac.threshold_, refined_inliers);;

      for (const auto& inlier : refined_inliers) {
      inlier_track_ids.push_back(shared_track_ids[inlier]);
      }

      T_w_c = Sophus::SE3d(non_linear_transfomation.block<3,3>(0,0), non_linear_transfomation.block<3,1>(0,3));
}

struct BundleAdjustmentOptions {
  /// 0: silent, 1: ceres brief report (one line), 2: ceres full report
  int verbosity_level = 1;

  /// update intrinsics or keep fixed
  bool optimize_intrinsics = false;

  /// use huber robust norm or squared norm
  bool use_huber = true;

  /// parameter for huber loss (in pixel)
  double huber_parameter = 1.0;

  /// maximum number of solver iterations
  int max_num_iterations = 22;

  /// imu optimization weight
  double imu_optimization_weight = 0.4;
};

// Run bundle adjustment to optimize cameras, points, and optionally intrinsics
void Proj_bundle_adjustment(const Corners& feature_corners,
                       const BundleAdjustmentOptions& options,
                       const std::set<FrameCamId>& fixed_cameras,
                       Calibration& calib_cam, Cameras& cameras,
                       Landmarks& landmarks) {
  ceres::Problem problem;

// TODO SHEET 4: Setup optimization problem
  // UNUSED(feature_corners);
  // UNUSED(options);
  // UNUSED(fixed_cameras);
  // UNUSED(calib_cam);
  // UNUSED(cameras);
  // UNUSED(landmarks);
for (auto& [track_id, landmark] : landmarks) {
    for (auto& [fcid, feature_id] : landmark.obs) {
      // corresponding cameras and feature points
      if (cameras.find(fcid) == cameras.end() || feature_corners.find(fcid) == feature_corners.end()) {
        continue; }

      const Eigen::Vector2d& p_2d = feature_corners.at(fcid).corners[feature_id];
      Eigen::Vector3d& p_3d_w = landmark.p;
      std::string cam_model = calib_cam.intrinsics[fcid.cam_id]->name();

      // set up residuals blocks
      BundleAdjustmentReprojectionCostFunctor* c = new BundleAdjustmentReprojectionCostFunctor(p_2d, cam_model);
      ceres::CostFunction* cost_function = new ceres::AutoDiffCostFunction<
      BundleAdjustmentReprojectionCostFunctor, 2, 7, 3, 8>(c);

      // add residuals blocks
      problem.AddResidualBlock(cost_function, 
      options.use_huber ? new ceres::HuberLoss(options.huber_parameter) : nullptr,
      cameras[fcid].T_w_c.data(),
      p_3d_w.data(),
      calib_cam.intrinsics[fcid.cam_id]->data());

      problem.AddParameterBlock(cameras[fcid].T_w_c.data(), Sophus::SE3d::num_parameters,
      new Sophus::test::LocalParameterizationSE3);

      if (fixed_cameras.find(fcid) != fixed_cameras.end()) {
      problem.SetParameterBlockConstant(cameras[fcid].T_w_c.data());
      }
      if(!options.optimize_intrinsics){problem.SetParameterBlockConstant(calib_cam.intrinsics[fcid.cam_id]->data());}
      

}
}
  // Solve
  ceres::Solver::Options ceres_options;
  ceres_options.max_num_iterations = options.max_num_iterations;
  ceres_options.linear_solver_type = ceres::SPARSE_SCHUR;
  ceres_options.num_threads = std::thread::hardware_concurrency();
  ceres::Solver::Summary summary;
  Solve(ceres_options, &problem, &summary);
  switch (options.verbosity_level) {
    // 0: silent
    case 1:
      std::cout << summary.BriefReport() << std::endl;
      break;
    case 2:
      std::cout << summary.FullReport() << std::endl;
      break;
  }
}

// Run bundle adjustment to optimize cameras, points, and optionally intrinsics
void Imu_Proj_bundle_adjustment(
  const Corners& feature_corners, const BundleAdjustmentOptions& options,
  const std::set<FrameCamId>& fixed_cameras, Calibration& calib_cam,
  Cameras& cameras, Landmarks& landmarks,
  Eigen::aligned_map<Timestamp, PoseVelState<double>>& states,
  Eigen::aligned_map<Timestamp, IntegratedImuMeasurement<double>>&
      imu_measurements,
  std::vector<Timestamp>& timestamps) {
  // set up ceres problem
  ceres::Problem problem;

//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
  // std::cout<< "weight : " << options.imu_optimization_weight << std::endl;
  for (auto& cam : cameras) {
    problem.AddParameterBlock(cam.second.T_w_c.data(),
                              Sophus::SE3d::num_parameters,
                              new Sophus::test::LocalParameterizationSE3);
    // Set the fixed frame paramter constant
    if (fixed_cameras.find(cam.first) != fixed_cameras.end()) {
      problem.SetParameterBlockConstant(cam.second.T_w_c.data());
    }
  }

  //std::cout<< "Add states parameter block!!!"<<std::endl;
  // add data imu (state of IMU) to ResidualsBlock
  for (auto& state : states) {
    problem.AddParameterBlock(state.second.T_w_i.data(),
                              Sophus::SE3d::num_parameters,
                              new Sophus::test::LocalParameterizationSE3);
  }

//////////////////////////////////////////////////////////////
//std::cout<< "Add camera and states residuals block !"<<std::endl;
//////////////////////////////////////////////////////////////
  
  for (auto& [track_id, landmark] : landmarks) {
    auto& p_3d = landmark.p;
    for (auto& [fcid, feature_id] : landmark.obs) {

      const auto& p_2d = feature_corners.at(fcid).corners[feature_id];
      BundleAdjustmentReprojectionCostFunctor* c =
          new BundleAdjustmentReprojectionCostFunctor(
              p_2d, calib_cam.intrinsics[fcid.cam_id]->name());
      ceres::CostFunction* cost_function = new ceres::AutoDiffCostFunction<
          BundleAdjustmentReprojectionCostFunctor, 2,
          Sophus::SE3d::num_parameters, 3, 8>(c);

      if (cameras.count(fcid)) { 
          // 'cameras list' has frame&camera id then optimize camera location
        problem.AddResidualBlock(
            cost_function,
            options.use_huber ? new ceres::HuberLoss(options.huber_parameter) : nullptr,
            cameras[fcid].T_w_c.data(), p_3d.data(),
            calib_cam.intrinsics[fcid.cam_id]->data());
      } 
      else
      {std::cout<< fcid << " isn't in Cameras!!!"<<std::endl;}
    }
  }

//////////////////////////////////////////////////////////////
 //std::cout<< "KF cameras BA with IMU Measurement !"<<std::endl;
//////////////////////////////////////////////////////////////

  for(auto& state: states){
    auto it = std::find(timestamps.begin(), timestamps.end(), state.first);
    if (it != timestamps.end()) {
        int Fid = std::distance(timestamps.begin(), it);
 
        FrameCamId camlid(Fid, 0);
        //std::cout << "Found corresponding FramdcamID: "<< camlid << std::endl;

        if(cameras.count(camlid)){
          BundleAdjustmentImuCamstateCostFunctor* cam_imu_cost =
              new BundleAdjustmentImuCamstateCostFunctor(
                   calib_cam.T_i_c[0]);
  
          ceres::CostFunction* cost_function = new ceres::AutoDiffCostFunction<
              BundleAdjustmentImuCamstateCostFunctor, Sophus::SE3d::DoF,
              Sophus::SE3d::num_parameters,
              Sophus::SE3d::num_parameters
              >(cam_imu_cost);

          ceres::LossFunction* loss_function = nullptr;

          if (options.use_huber) {
            loss_function = new ceres::HuberLoss(options.huber_parameter);

          }  
          problem.AddResidualBlock(
              cost_function,
              new ceres::ScaledLoss(loss_function, options.imu_optimization_weight, ceres::TAKE_OWNERSHIP), // introduce weight for effect of IMU BA
              cameras[camlid].T_w_c.data(), state.second.T_w_i.data()
          );
        }
    } else {
        std::cout << "FramdcamID is fixed Camid from previous KF"<< std::endl;
    }
  }
  

//////////////////////////////////////////////////////////////
  //std::cout<< "imu BA ing!"<<std::endl;
//////////////////////////////////////////////////////////////
  // Add the parameter block first
  if (states.size() >= 3) {
    //reset
    auto iter = states.rbegin();
    std::size_t iter_counter = 0;

    while (iter_counter < (states.size() - 1)) {
      const IntegratedImuMeasurement<double>& imu_meas = imu_measurements[iter->first];
      visnav::PoseVelState<double>& state1 = states[iter->first];
      double st1 = (iter->first) * 1e-9;
      ++iter;
      double st0 = (iter->first) * 1e-9;
      visnav::PoseVelState<double>& state0 = states[iter->first];
      double diff_t = st1 - st0;
      //std::cout<< "time difference : " << diff_t << std::endl;
      // if(diff_t > 2.8 || diff_t < 1e-4){
      //   //std::cout<< "The interval of consecutive keyframes is unexpected !!! Optimization skipping ! "<< std::endl;
      //   continue;
      // }

      BundleAdjustmentImuCostFunctor* imu_c = new BundleAdjustmentImuCostFunctor(
          imu_meas.getDeltaState(), visnav::constants::g, state0.t_ns, state1.t_ns,
          calib_cam.T_i_c[0]);
      ceres::CostFunction* imu_cost_function = new ceres::AutoDiffCostFunction<
          BundleAdjustmentImuCostFunctor, 9,
          Sophus::SE3d::num_parameters,  // state0.T_w_i
          Sophus::SE3d::num_parameters,  // state1.T_w_i
          3,                             // state0.vel_w_i
          3                              // state1.vel_w_i                            
          >(imu_c);

      ceres::LossFunction* loss_function = nullptr;

      if (options.use_huber) {
        loss_function = new ceres::HuberLoss(options.huber_parameter);

      }
      problem.AddResidualBlock(
          imu_cost_function, 
          new ceres::ScaledLoss(loss_function, options.imu_optimization_weight, ceres::TAKE_OWNERSHIP), // introduce weight for effect of IMU BA
          state0.T_w_i.data(), state1.T_w_i.data(), state0.vel_w_i.data(),
          state1.vel_w_i.data());
      
      iter_counter++;

    }
  }
  else{
    std::cout<<" The num of processed states is smaller than 3 !!! "<<std::endl;
  }

  if (!options.optimize_intrinsics) {
    // Keep the intrinsics fixed
    problem.SetParameterBlockConstant(calib_cam.intrinsics[0]->data());
    problem.SetParameterBlockConstant(calib_cam.intrinsics[1]->data());
  } else {
    // Do nothing
  }

  // Solve
  ceres::Solver::Options ceres_options;
  ceres_options.max_num_iterations = options.max_num_iterations;
  ceres_options.linear_solver_type = ceres::SPARSE_SCHUR;
  ceres_options.num_threads = std::thread::hardware_concurrency();
  ceres::Solver::Summary summary;
  Solve(ceres_options, &problem, &summary);
  switch (options.verbosity_level) {
    // 0: silent
    case 1:
      std::cout << summary.BriefReport() << std::endl;
      break;
    case 2:
      std::cout << summary.FullReport() << std::endl;
      break;
  }
}

void take_framestates(
    const Calibration& calib_cam,
    Cameras& cameras,
    std::vector<Timestamp>& timestamps,
    PoseVelState<double>& frame_state,
    Eigen::aligned_map<Timestamp, PoseVelState<double>>& frame_states,
    Eigen::aligned_map<Timestamp, PoseVelState<double>>& frame_states_opt) {
  frame_states_opt.clear();
  for (const auto& kv : cameras) {

      frame_state.T_w_i = kv.second.T_w_c * calib_cam.T_i_c[0].inverse(); // here we load T_w_c, we will do next transformation for the cost function
      frame_state.vel_w_i = frame_states[timestamps[kv.first.frame_id]].vel_w_i;
      frame_state.t_ns = timestamps[kv.first.frame_id];
      frame_states_opt[timestamps[kv.first.frame_id]] = frame_state;  
  }
}

void update_framestates(
    const Calibration& calib_cam,
    Cameras& cameras,
    std::vector<Timestamp>& timestamps,
    Eigen::aligned_map<Timestamp, PoseVelState<double>>& frame_states,
    Eigen::aligned_map<Timestamp, PoseVelState<double>>& frame_states_opt) {
  for (auto& it : frame_states_opt) {
    frame_states[it.first] = it.second;
  }
  for(const auto& cam : cameras){
      frame_states[timestamps[cam.first.frame_id]].T_w_i = cam.second.T_w_c * calib_cam.T_i_c[0].inverse();
  }
}



}  // namespace visnav
