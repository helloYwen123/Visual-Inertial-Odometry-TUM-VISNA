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

#include <set>

#include <visnav/common_types.h>

#include <visnav/calibration.h>

#include <opengv/absolute_pose/CentralAbsoluteAdapter.hpp>
#include <opengv/absolute_pose/methods.hpp>
#include <opengv/relative_pose/CentralRelativeAdapter.hpp>
#include <opengv/sac/Ransac.hpp>
#include <opengv/sac_problems/absolute_pose/AbsolutePoseSacProblem.hpp>
#include <opengv/triangulation/methods.hpp>

namespace visnav {

void project_landmarks(
    const Sophus::SE3d& current_pose,
    const std::shared_ptr<AbstractCamera<double>>& cam,
    const Landmarks& landmarks, const double cam_z_threshold,
    std::vector<Eigen::Vector2d, Eigen::aligned_allocator<Eigen::Vector2d>>&
        projected_points,
    std::vector<TrackId>& projected_track_ids) {
  projected_points.clear();
  projected_track_ids.clear();

// TODO SHEET 5: project landmarks to the image plane using the current
// locations of the cameras. Put 2d coordinates of the projected points into
// projected_points and the corresponding id of the landmark into
// projected_track_ids.

  // Extract the rotation (R) and translation (T) from the current_pose

  for (const auto& [track_id, landmark] : landmarks) {
      // Transform landmark to camera coordinates
    Eigen::Vector3d P_c = current_pose.inverse() * landmark.p;
      // Ignore points behind the camera
      if (P_c.z() < cam_z_threshold) {
          continue;
      }

      // Project the 3D point to 2D using the camera model
      auto p_i = cam->project(P_c);
      

      auto image_width = cam->width();
      auto image_height = cam->height();

      // Store the valid projection
      if (p_i.x() >= 0 && p_i.x() < image_width && p_i.y() >= 0 && p_i.y() < image_height) {
      projected_points.push_back(p_i);
      projected_track_ids.push_back(track_id);}
  }

}

void find_matches_landmarks(
    const KeypointsData& kdl, const Landmarks& landmarks,
    const Corners& feature_corners,
    const std::vector<Eigen::Vector2d,
                      Eigen::aligned_allocator<Eigen::Vector2d>>&
        projected_points,
    const std::vector<TrackId>& projected_track_ids,
    const double match_max_dist_2d, const int feature_match_threshold,
    const double feature_match_dist_2_best, LandmarkMatchData& md) {
  md.matches.clear();

// TODO SHEET 5: Find the matches between projected landmarks and detected
// keypoints in the current frame. For every detected keypoint search for
// matches inside a circle with radius match_max_dist_2d around the point
// location. For every landmark the distance is the minimal distance between the
// descriptor of the current point and descriptors of all observations of the
// landmarks. The feature_match_threshold and feature_match_dist_2_best should
// be used to filter outliers the same way as in exercise 3. You should fill
// md.matches with <featureId,trackId> pairs for the successful matches that
// pass all tests.
  // UNUSED(kdl);
  // UNUSED(landmarks);
  // UNUSED(feature_corners);
  // UNUSED(projected_points);
  // UNUSED(projected_track_ids);
  // UNUSED(match_max_dist_2d);
  // UNUSED(feature_match_threshold);
  // UNUSED(feature_match_dist_2_best);

   for (size_t i = 0; i < kdl.corners.size(); ++i) {
    const Eigen::Vector2d& keypoint = kdl.corners[i];
    const auto& descriptor = kdl.corner_descriptors[i];
    int best_dist = INT_MAX;
    int second_best_dist = INT_MAX;
    TrackId best_track_id = -1;

    //std::cout << "Keypoint " << i << ": " << keypoint.transpose() << std::endl;

    for (size_t j = 0; j < projected_points.size(); ++j) {
      const Eigen::Vector2d& projected_point = projected_points[j];
      TrackId track_id = projected_track_ids[j];

      if ((keypoint - projected_point).norm() <= match_max_dist_2d) {
          const Landmark& landmark = landmarks.at(track_id);

          // Find the minimum distance for the current landmark
          int min_landmark_dist = INT_MAX;
          for (const auto& [frame_cam_id, feature_id] : landmark.obs) {
            const KeypointsData& kd = feature_corners.at(frame_cam_id);
            const auto& landmark_descriptor = kd.corner_descriptors[feature_id];
            int dist = (descriptor ^ landmark_descriptor).count();
            // std::cout << "  Projected point " << j << ": " << projected_point.transpose()
            // << ", Distance: " << dist << ", Track ID: " << track_id 
            // << " landmark's frame cam id: "<< frame_cam_id<< " feature id: "<< feature_id << std::endl;

            if (dist < min_landmark_dist) {
              min_landmark_dist = dist;
            }
          }

          if (min_landmark_dist < best_dist) {
            second_best_dist = best_dist;
            best_dist = min_landmark_dist;
            best_track_id = track_id;
          } else if (min_landmark_dist < second_best_dist) {
            second_best_dist = min_landmark_dist;
          }
        }
      }

    if(best_dist < feature_match_threshold &&
        second_best_dist >= best_dist * feature_match_dist_2_best) {
      md.matches.emplace_back(i, best_track_id);
    //   std::cout << "Match found: Keypoint " << i << " matched with Track " << best_track_id 
    //             << " - Best distance: " << best_dist
    //             << ", Second best distance: " << second_best_dist << std::endl;
    // } else {
    //   std::cout << "Match discarded: Keypoint " << i << " - Best distance: " << best_dist
    //             << ", Second best distance: " << second_best_dist << std::endl;
    // }
    }
    }
  }
  

void localize_camera(const Sophus::SE3d& current_pose,
                     const std::shared_ptr<AbstractCamera<double>>& cam,
                     const KeypointsData& kdl, const Landmarks& landmarks,
                     const double reprojection_error_pnp_inlier_threshold_pixel,
                     LandmarkMatchData& md) {
  md.inliers.clear();

  // default to previous pose if not enough inliers
  md.T_w_c = current_pose;

  if (md.matches.size() < 4) {
    return;
  }

// TODO SHEET 5: Find the pose (md.T_w_c) and the inliers (md.inliers) using the
// landmark to keypoints matches and PnP. This should be similar to the
// localize_camera in exercise 4 but in this exercise we don't explicitly have
// tracks.
  // UNUSED(cam);
  // UNUSED(kdl);
  // UNUSED(landmarks);
  // UNUSED(reprojection_error_pnp_inlier_threshold_pixel);
 // Extract 2D-3D correspondences
  std::vector<Eigen::Vector3d> points3d;
  std::vector<Eigen::Vector2d> points2d;
  for (const auto& match : md.matches) {
    const FeatureId& feature_id = match.first;
    const TrackId& track_id = match.second;

    if (landmarks.find(track_id) != landmarks.end()) {
      const Landmark& landmark = landmarks.at(track_id);
      points3d.push_back(landmark.p);
      points2d.push_back(kdl.corners[feature_id]);
    }
  }

  // Prepare bearing vectors and 3D points for PnP
  opengv::bearingVectors_t bearingVectors;
  opengv::points_t points;
  for (size_t i = 0; i < points2d.size(); ++i) {
    Eigen::Vector3d bearing = cam->unproject(points2d[i]).normalized(); //3d
    bearingVectors.push_back(bearing);
    points.push_back(points3d[i]);
  }

  // Set up the PnP adapter
  opengv::absolute_pose::CentralAbsoluteAdapter adapter(bearingVectors, points);

  // Set up the RANSAC problem
  typedef opengv::sac_problems::absolute_pose::AbsolutePoseSacProblem AbsolutePoseSacProblem;
  std::shared_ptr<AbsolutePoseSacProblem> absposeproblem_ptr(
    new AbsolutePoseSacProblem(adapter, AbsolutePoseSacProblem::KNEIP));

  opengv::sac::Ransac<AbsolutePoseSacProblem> ransac;
  ransac.sac_model_ = absposeproblem_ptr;
  ransac.threshold_ = 1.0 - std::cos(std::atan(reprojection_error_pnp_inlier_threshold_pixel / 500.0));
  ransac.max_iterations_ = 1000;

  if (!ransac.computeModel()) {
    return;
  }

  // Non-linear optimization
  adapter.sett(ransac.model_coefficients_.block<3,1>(0,3));
  adapter.setR(ransac.model_coefficients_.block<3,3>(0,0));

  opengv::transformation_t non_linear_transformation = opengv::absolute_pose::optimize_nonlinear(adapter, ransac.inliers_);

  // Re-evaluate inliers
  std::vector<int> refined_inliers;
  absposeproblem_ptr->selectWithinDistance(non_linear_transformation, ransac.threshold_, refined_inliers);

  for (const auto& inlier : refined_inliers) {
    md.inliers.push_back(md.matches[inlier]);
  }

  // Update the pose
  md.T_w_c = Sophus::SE3d(non_linear_transformation.block<3,3>(0,0), non_linear_transformation.block<3,1>(0,3));
}


void add_new_landmarks(const FrameCamId fcidl, const FrameCamId fcidr,
                       const KeypointsData& kdl, const KeypointsData& kdr,
                       const Calibration& calib_cam, const MatchData& md_stereo,
                       const LandmarkMatchData& md, Landmarks& landmarks,
                       TrackId& next_landmark_id) {
  // input should be stereo pair
  assert(fcidl.cam_id == 0);
  assert(fcidr.cam_id == 1);

  const Sophus::SE3d T_0_1 = calib_cam.T_i_c[0].inverse() * calib_cam.T_i_c[1];
  const Eigen::Vector3d t_0_1 = T_0_1.translation();
  const Eigen::Matrix3d R_0_1 = T_0_1.rotationMatrix();

  // TODO SHEET 5: Add new landmarks and observations. Here md_stereo contains
  // stereo matches for the current frame and md contains feature to landmark
  // matches for the left camera (camera 0). For all inlier feature to landmark
  // matches add the observations to the existing landmarks. If the left
  // camera's feature appears also in md_stereo.inliers, then add both
  // observations. For all inlier stereo observations that were not added to the
  // existing landmarks, triangulate and add new landmarks. Here
  // next_landmark_id is a running index of the landmarks, so after adding a new
  // landmark you should always increase next_landmark_id by 1.
  for (auto& kv : md.inliers) {  // In this frame
    const FeatureId& f_id = kv.first;
    const TrackId& t_id = kv.second;
    if (landmarks.count(t_id) > 0)  // landmark exists
    {
      landmarks.at(t_id).obs.emplace(std::make_pair(fcidl, f_id));

      // Check if feature id also exist in stereo pair
      for (auto& inlier_pair : md_stereo.inliers) {
        if (inlier_pair.first == f_id) {
          landmarks.at(t_id).obs.emplace(
              std::make_pair(fcidr, inlier_pair.second));
          break;
        }
      }
    } else {
    }
  }
  for (auto& kv : md_stereo.inliers) {
    const FeatureId& f_idl = kv.first;
    const FeatureId& f_idr = kv.second;
    int counter = 0;
    for (auto& fid_tid : md.inliers) {
      const FeatureId& fid = fid_tid.first;
      if (fid == f_idl) {
        // Already exist
        counter++;
        break;
      } else {
      }
    }
    if (counter == 0) {
      // DO TRIANGULATION
      opengv::bearingVectors_t bearingVectors1;
      opengv::bearingVectors_t bearingVectors2;

      bearingVectors1.push_back(
          calib_cam.intrinsics[fcidl.cam_id]->unproject(kdl.corners.at(f_idl)));
      bearingVectors2.push_back(
          calib_cam.intrinsics[fcidr.cam_id]->unproject(kdr.corners.at(f_idr)));

      opengv::relative_pose::CentralRelativeAdapter adapter(
          bearingVectors1, bearingVectors2, t_0_1, R_0_1);
      opengv::point_t point =
          md.T_w_c * opengv::triangulation::triangulate(adapter, 0);
      Landmark l;
      l.p = point;
      l.obs.emplace(std::make_pair(fcidl, f_idl));
      l.obs.emplace(std::make_pair(fcidr, f_idr));
      landmarks.emplace(std::make_pair(next_landmark_id++, l));
    }
  }
}

bool delete_oldframes(const FrameCamId fcidl, const int max_num_kfs,
                          Cameras& cameras, Landmarks& landmarks,
                          Landmarks& old_landmarks,
                          std::set<FrameId>& kf_frames, Camera& removed_camera,
                          FrameId& removed_fid) {
  kf_frames.emplace(fcidl.frame_id);
  
  bool removed = false;   // remove elements from three containers : 1landmarks; 2cameras ; 3kf_frames;
// TODO SHEET 5: Remove old cameras and observations if the number of keyframe
// pairs (left and right image is a pair) is larger than max_num_kfs. The ids of
// all the keyframes that are currently in the optimization should be stored in
// kf_frames. Removed keyframes should be removed from cameras and landmarks
// with no left observations should be moved to old_landmarks.
  // UNUSED(max_num_kfs);
  // UNUSED(cameras);
  // UNUSED(landmarks);
  // UNUSED(old_landmarks);
  // add current key into set of keyframes
  
  // check and keep the num of keyframes
  while (static_cast<int>(kf_frames.size()) > max_num_kfs) {
    // find and remove the oldest keyframe

    //std::cout<<"remove a camera from Cameras!"<<std::endl;
    removed = true;
    FrameId oldest_frame_id = *kf_frames.begin();
    removed_fid = oldest_frame_id;
    kf_frames.erase(oldest_frame_id);

    //std::cout<< "erase once keyframe!!"<<std::endl;

    // set up FrameCamId
    FrameCamId left_cam_id(oldest_frame_id, 0);
    FrameCamId right_cam_id(oldest_frame_id, 1);

    // remove the camera
    FrameCamId rfcid(removed_fid, 0);
    removed_camera = cameras.at(rfcid);
    cameras.erase(left_cam_id);
    cameras.erase(right_cam_id);

    // traverse landmarks and delete obervation
    for (auto it = landmarks.begin(); it != landmarks.end();) {
      it->second.obs.erase(left_cam_id);
      it->second.obs.erase(right_cam_id);

      if (it->second.obs.empty()) {
        // move landmarks with no oberservation into old_landmarks
        old_landmarks[it->first] = std::move(it->second);
        it = landmarks.erase(it); //delete iterator and update it 
      } else {
        ++it;
      }
    }
  }

  //if (removed){std::cout << "KF num is larger than threshold! "<< " remove KF id : "<< removed_fid << std::endl;}

  return removed;
}
}  // namespace visnav
