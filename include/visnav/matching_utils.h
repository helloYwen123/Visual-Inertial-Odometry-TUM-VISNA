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

#include <bitset>
#include <set>

#include <Eigen/Dense>
#include <sophus/se3.hpp>

#include <opengv/relative_pose/CentralRelativeAdapter.hpp>
#include <opengv/relative_pose/methods.hpp>
#include <opengv/sac/Ransac.hpp>
#include <opengv/sac_problems/relative_pose/CentralRelativePoseSacProblem.hpp>

#include <visnav/camera_models.h>
#include <visnav/common_types.h>

namespace visnav {

void computeEssential(const Sophus::SE3d& T_0_1, Eigen::Matrix3d& E) {
  const Eigen::Vector3d t_0_1 = T_0_1.translation().normalized();
  const Eigen::Matrix3d R_0_1 = T_0_1.rotationMatrix();

// TODO SHEET 3: compute essential matrix
  // UNUSED(E);
  // UNUSED(t_0_1);
  // UNUSED(R_0_1);
  Eigen::Matrix3d T_hat;
  T_hat << 0, -t_0_1.z(), t_0_1.y(),
           t_0_1.z(), 0, -t_0_1.x(),
           -t_0_1.y(), t_0_1.x(), 0;
  E = T_hat * R_0_1;
}

void findInliersEssential(const KeypointsData& kd1, const KeypointsData& kd2,
                          const std::shared_ptr<AbstractCamera<double>>& cam1,
                          const std::shared_ptr<AbstractCamera<double>>& cam2,
                          const Eigen::Matrix3d& E,
                          double epipolar_error_threshold, MatchData& md) {
  md.inliers.clear();   
  for (size_t j = 0; j < md.matches.size(); j++) {
    const Eigen::Vector2d p0_2d = kd1.corners[md.matches[j].first];
    const Eigen::Vector2d p1_2d = kd2.corners[md.matches[j].second];

// TODO SHEET 3: determine inliers and store in md.inliers
    // UNUSED(cam1);
    // UNUSED(cam2);
    // UNUSED(E);
    // UNUSED(epipolar_error_threshold);
    // UNUSED(p0_2d);
    // UNUSED(p1_2d);
    Eigen::Vector3d p0_3d = cam1->unproject(p0_2d);
    Eigen::Vector3d p1_3d = cam2->unproject(p1_2d);
    double Essential_constraint =  p0_3d.transpose() * E * p1_3d;
    //std::cout<< " essential contrainst: "<< Essential_constraint<<std::endl;
    
    if(std::abs(Essential_constraint) <= epipolar_error_threshold)  {
      md.inliers.push_back(md.matches[j]);
      }
   }
 }

void findInliersRansac(const KeypointsData& kd1, const KeypointsData& kd2,
                       const std::shared_ptr<AbstractCamera<double>>& cam1,
                       const std::shared_ptr<AbstractCamera<double>>& cam2,
                       const double ransac_thresh, const int ransac_min_inliers,
                       MatchData& md) {
  md.inliers.clear();
  md.T_i_j = Sophus::SE3d();

// TODO SHEET 3: Run RANSAC with using opengv's CentralRelativePose and store
// the final inlier indices in md.inliers and the final relative pose in
// md.T_i_j (normalize translation). If the number of inliers is smaller than
// ransac_min_inliers, leave md.inliers empty. Note that if the initial RANSAC
// was successful, you should do non-linear refinement of the model parameters
// using all inliers, and then re-estimate the inlier set with the refined
// model parameters.
  // UNUSED(kd1);
  // UNUSED(kd2);
  // UNUSED(cam1);
  // UNUSED(cam2);
  // UNUSED(ransac_thresh);
  // UNUSED(ransac_min_inliers);
  opengv::bearingVectors_t bearingVectors1;  //observation vectors
  opengv::bearingVectors_t bearingVectors2;

  for (const auto& match : md.matches) {
        bearingVectors1.push_back(cam1->unproject(kd1.corners[match.first]));
        bearingVectors2.push_back(cam2->unproject(kd2.corners[match.second])); //ransac deal with the bearingVectors
    }

  opengv::relative_pose::CentralRelativeAdapter adapter(bearingVectors1, bearingVectors2);  //set up retative adapter
  opengv::sac::Ransac<opengv::sac_problems::relative_pose::CentralRelativePoseSacProblem> ransac; //set up ransac
  std::shared_ptr<opengv::sac_problems::relative_pose::CentralRelativePoseSacProblem> relposeproblem_ptr(
      new opengv::sac_problems::relative_pose::CentralRelativePoseSacProblem(
          adapter, opengv::sac_problems::relative_pose::CentralRelativePoseSacProblem::NISTER));
  
  // run ransac
  ransac.sac_model_ = relposeproblem_ptr;
  ransac.threshold_ = ransac_thresh;
  ransac.max_iterations_ = 1000;
  if(!ransac.computeModel()){return;};

  opengv::transformation_t best_transformation = ransac.model_coefficients_;
  std::vector<int> inliers;
  relposeproblem_ptr->selectWithinDistance(best_transformation, ransac_thresh, inliers);

  if(inliers.size()< static_cast<size_t>(ransac_min_inliers)){
      md.inliers.clear();
      return;}

  auto refined_model = opengv::relative_pose::optimize_nonlinear(adapter, inliers);

  //re-evaluate the inliers
  std::vector<int> refined_inliers;
  relposeproblem_ptr->selectWithinDistance(refined_model, ransac_thresh, refined_inliers);

  md.inliers.clear();
  for (const auto& idx : refined_inliers) {
      md.inliers.push_back(md.matches[idx]);
  }

  Eigen::Matrix4d refined_T_Matrix = Eigen::Matrix4d::Identity();
  refined_T_Matrix.block<3,4>(0,0) = refined_model;
  md.T_i_j = Sophus::SE3d(refined_T_Matrix);

  //do normalization
  md.T_i_j.translation() = md.T_i_j.translation().normalized();

}

}  // namespace visnav
