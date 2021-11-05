/* Copyright (c) 2017, United States Government, as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 *
 * All rights reserved.
 *
 * The Astrobee platform is licensed under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with the
 * License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
#ifndef CALIBRATION_TEST_UTILITIES_H_
#define CALIBRATION_TEST_UTILITIES_H_

#include <ff_common/eigen_vectors.h>
#include <localization_common/image_correspondences.h>

#include <vector>

namespace calibration {
class RegistrationCorrespondences {
  RegistrationCorrespondences();

  const localization_common::ImageCorrespondences& correspondences() const { return correspondences_; }

  const Eigen::Isometry3d& camera_T_target() const { return camera_T_target_; }

  static std::vector<Eigen::Vector3d> TargetPoints();

 private:
  localization_common::ImageCorrespondences correspondences_;
  Eigen::Isometry3d camera_T_target_;
  Eigen::Matrix3d intrinsics_;
};
}  // namespace calibration

#endif  // CALIBRATION_TEST_UTILITIES_H_
