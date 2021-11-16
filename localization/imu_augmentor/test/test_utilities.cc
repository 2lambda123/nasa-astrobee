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

#include "test_utilities.h"  // NOLINT
#include <localization_common/logger.h>

namespace imu_augmentor {
namespace lc = localization_common;
namespace lm = localization_measurements;

ImuAugmentorParams DefaultImuAugmentorParams() {
  ImuAugmentorParams params;
  params.gravity = gtsam::Vector3::Zero();
  params.body_T_imu = gtsam::Pose3::identity();
  // Filer params are already default none
  params.gyro_sigma = 0.1;
  params.accel_sigma = 0.1;
  params.accel_bias_sigma = 0.1;
  params.gyro_bias_sigma = 0.1;
  params.integration_variance = 0.1;
  params.bias_acc_omega_int = 0.1;
  params.standstill_enabled = true;
}

std::vector<lm::ImuMeasurement> ConstantAccelerationMeasurements(const gtsam::Vector3& acceleration,
                                                                 const int num_measurements, const lc::Time start_time,
                                                                 const double time_increment) {
  const Eigen::Vector3d angular_velocity(Eigen::Vector3d::Zero());
  std::vector<lm::ImuMeasurement> imu_measurements;
  for (int i = 0; i < num_measurements; ++i) {
    const lc::Time time = start_time + i * time_increment;
    imu_measurements.emplace_back(lm::ImuMeasurement(acceleration, angular_velocity, time));
  }
  return imu_measurements;
}
}  // namespace imu_augmentor
