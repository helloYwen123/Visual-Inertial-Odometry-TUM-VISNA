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

#include <sophus/se3.hpp>

#include <visnav/common_types.h>

namespace visnav {

// Implement exp for SO(3)
template <class T>
Eigen::Matrix<T, 3, 3> user_implemented_expmap(
    const Eigen::Matrix<T, 3, 1>& xi) {
// TODO SHEET 1: implement
  T theta = xi.norm();
  Eigen::Matrix<T, 3, 3> I = Eigen::Matrix<T, 3, 3>::Identity();
  if(theta < std::numeric_limits<T>::epsilon()) {
    return I;
  }
  Eigen::Matrix<T, 3, 1> a = xi.normalized();
  Eigen::Matrix<T, 3, 3> a_hat;

  a_hat << 0, -a.z() , a.y(), 
          a.z(), 0, -a.x(),
          -a.y(), a.x(), 0 ;


  return I + std::sin(theta) * a_hat  + (1 - std::cos(theta)) * a_hat * a_hat;
}

// Implement log for SO(3)
template <class T>
Eigen::Matrix<T, 3, 1> user_implemented_logmap(
    const Eigen::Matrix<T, 3, 3>& mat) {
// TODO SHEET 1: implement
  T trace = mat.trace();
  Eigen::Matrix<T,3,1> omega;
  T theta = std::acos((trace - 1) / 2.0);
  if(theta < std::numeric_limits<T>::epsilon()){return Eigen::Matrix<T, 3, 1>::Zero();}  
  omega << (mat(2,1) - mat(1,2)) , (mat(0,2) -mat(2,0)) , (mat(1,0) - mat(0,1));
  omega = omega * (theta / (2*std::sin(theta))); // divide 2 a intergrate  , cause datatype converting
  return omega;
}

// Implement exp for SE(3)
template <class T>
Eigen::Matrix<T, 4, 4> user_implemented_expmap(
    const Eigen::Matrix<T, 6, 1>& xi) {
// TODO SHEET 1: implement
  Eigen::Matrix<T, 3, 1> Ta = xi.template head<3>();
  Eigen::Matrix<T, 3, 1> Ro = xi.template tail<3>();
  T theta = Ro.norm();
  Eigen::Matrix<T,3,3> J;
  Eigen::Matrix<T, 3, 3> Ro_hat;
  Eigen::Matrix<T, 4, 4> SE = Eigen::Matrix<T, 4 , 4>::Identity();
  Ro_hat <<  0, -Ro.z() , Ro.y(), 
          Ro.z(), 0, -Ro.x(),
          -Ro.y(), Ro.x(), 0 ;
  if( theta < std::numeric_limits<T>::epsilon() )
    {
       J = Eigen::Matrix<T,3,3>::Identity();
    } 
  else
    {
      J = Eigen::Matrix<T, 3, 3>::Identity() + (1 - std::cos(theta)) * Ro_hat / (theta * theta)  +  Ro_hat * Ro_hat * (theta - std::sin(theta)) / (theta * theta * theta);
    }
  SE.template topLeftCorner<3, 3> () = user_implemented_expmap(Ro);
  SE.template topRightCorner<3, 1> () = J * Ta ;
  return SE;
}

// Implement log for SE(3)
template <class T>
Eigen::Matrix<T, 6, 1> user_implemented_logmap(
    const Eigen::Matrix<T, 4, 4>& mat) {
// TODO SHEET 1: implement
   Eigen::Matrix<T, 6, 1> xi = Eigen::Matrix<T,6,1>::Zero();
   Eigen::Matrix<T, 3, 3> R = mat.template topLeftCorner<3, 3>();
   Eigen::Matrix<T, 3, 1> t = mat.template topRightCorner<3, 1>();

   auto omage = user_implemented_logmap(R);
   auto theta = omage.norm();
   Eigen::Matrix<T, 3, 3> J_inv;
   Eigen::Matrix<T,3,3> omage_hat;
   omage_hat <<  0, -omage.z() , omage.y(), 
          omage.z(), 0, -omage.x(),
          -omage.y(), omage.x(), 0 ;
    if (theta < std::numeric_limits<T>::epsilon())
    {
      J_inv = Eigen::Matrix<T,3,3>::Identity();
    }
    else
    {
      J_inv = Eigen::Matrix<T, 3, 3>::Identity() - omage_hat / 2.0 + (omage_hat * omage_hat) * ((1.0 / (theta * theta)) - ((1 + std::cos(theta)) / (2.0 * theta * std::sin(theta))));
    }
   xi.template head<3>() = J_inv * t; 
   xi.template tail<3>() = omage; 
  return xi;
}

}  // namespace visnav
