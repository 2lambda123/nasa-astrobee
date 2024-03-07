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

#include <localization_common/pose_with_covariance_interpolater.h>
#include <localization_common/utilities.h>

namespace localization_common {
template <>
PoseWithCovariance PoseWithCovarianceInterpolater::Interpolate(const PoseWithCovariance& a, const PoseWithCovariance& b,
                                                               const double alpha) const {
  return localization_common::Interpolate(a, b, alpha);
}

template <>
PoseWithCovariance PoseWithCovarianceInterpolater::Relative(const PoseWithCovariance& a,
                                                            const PoseWithCovariance& b) const {
  const Eigen::Isometry3d relative_pose = a.pose.inverse() * b.pose;
  // See https://gtsam.org/2021/02/23/uncertainties-part3.html
  // Adjoints
  // Uses convention w_T_a and w_T_b for poses, s.t. adj_w_a is the adjoint for the
  // w_T_a pose.
  const gtsam::Pose3 w_T_a = GtPose(a.pose);
  const gtsam::Pose3 w_T_b = GtPose(b.pose);
  const gtsam::Matrix6 adj_w_a = w_T_a.AdjointMap();
  const gtsam::Matrix6 adj_b_w = w_T_b.inverse().AdjointMap();
  // Helper terms
  const gtsam::Matrix6 h = adj_b_w * adj_w_a;
  const gtsam::Matrix6 h_t = h.transpose();
  // Compute covariance without correlation terms
  gtsam::Matrix6 relative_covariance = h * a.covariance * h_t + b.covariance;
  // Add correlation terms if they exist
  const auto covariance_a_b = a.correlation_covariances.Get(b.correlation_covariances.Oldest()->timestamp);
  if (covariance_a_b) {
    relative_covariance = relative_covariance - h * covariance_a_b->value - covariance_a_b->value.transpose() * h_t;
  }
  return PoseWithCovariance(relative_pose, relative_covariance);
}
}  // namespace localization_common
