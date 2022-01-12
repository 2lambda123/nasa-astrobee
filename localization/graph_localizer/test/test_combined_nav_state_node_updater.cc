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

#include <graph_localizer/graph_localizer.h>
#include <graph_localizer/graph_localizer_params.h>
#include <graph_localizer/test_utilities.h>
#include <localization_common/logger.h>
#include <localization_common/test_utilities.h>
#include <localization_common/utilities.h>
#include <localization_measurements/imu_measurement.h>

#include <gtsam/base/numericalDerivative.h>
#include <gtsam/inference/Symbol.h>
#include <gtsam/linear/NoiseModel.h>
#include <gtsam/slam/BetweenFactor.h>

#include <gtest/gtest.h>

namespace gl = graph_localizer;
namespace go = graph_optimizer;
namespace lc = localization_common;
namespace lm = localization_measurements;
namespace sym = gtsam::symbol_shorthand;

gl::CombinedNavStateGraphValuesParams DefaultCombinedNavStateGraphValuesParams() {
  gl::CombinedNavStateGraphValuesParams params;
  params.ideal_duration = 3;
  params.min_num_states = 3;
  params.max_num_states = 20;
  return params;
}

go::GraphOptimizerParams DefaultGraphOptimizerParams() {
  go::GraphOptimizerParams params;
  params.verbose = false;
  params.fatal_failures = false;
  params.print_factor_info = false;
  params.use_ceres_params = false;
  params.max_iterations = 4;
  params.marginals_factorization = "qr";
  params.add_marginal_factors = false;
  params.huber_k = 1.345;
  params.log_rate = 100;
  return params;
}

gl::CombinedNavStateNodeUpdaterParams DefaultCombinedNavStateNodeUpdaterParams() {
  gl::CombinedNavStateNodeUpdaterParams params;
  params.starting_prior_translation_stddev = 0.02;
  params.starting_prior_quaternion_stddev = 0.01;
  params.starting_prior_velocity_stddev = 0.01;
  params.starting_prior_accel_bias_stddev = 0.001;
  params.starting_prior_gyro_bias_stddev = 0.001;
  params.huber_k = 1.345;
  params.global_N_body_start =
    lc::CombinedNavState(gtsam::Pose3::identity(), gtsam::Velocity3::Zero(), gtsam::imuBias::ConstantBias(), 0.0);
  params.add_priors = true;
  params.graph_values = DefaultCombinedNavStateGraphValuesParams();
  params.threshold_bias_uncertainty = false;
  return params;
}

gl::GraphInitializerParams DefaultGraphInitializerParams() {
  gl::GraphInitializerParams params;
  params.global_T_body_start = gtsam::Pose3::identity();
  params.global_V_body_start = gtsam::Velocity3::Zero();
  params.num_bias_estimation_measurements = 1;
  params.start_time = 0;
  params.initial_imu_bias = gtsam::imuBias::ConstantBias();
  params.gravity = gtsam::Vector3::Zero();
  params.body_T_imu = gtsam::Pose3::identity();
  params.filter = imu_integration::ImuFilterParams();
  params.gyro_sigma = 0.001;
  params.accel_sigma = 0.001;
  params.accel_bias_sigma = 0.001;
  params.gyro_bias_sigma = 0.001;
  params.integration_variance = 0.001;
  params.bias_acc_omega_int = 0.001;
  return params;
}

gl::GraphLocalizerParams DefaultGraphLocalizerParams() {
  gl::GraphLocalizerParams params;
  params.combined_nav_state_node_updater = DefaultCombinedNavStateNodeUpdaterParams();
  params.graph_optimizer = DefaultGraphOptimizerParams();
  params.graph_initializer = DefaultGraphInitializerParams();
  params.huber_k = 1.345;
  return params;
}

gl::DepthOdometryFactorAdderParams DefaultDepthOdometryFactorAdderParams() {
  gl::DepthOdometryFactorAdderParams params;
  params.noise_scale = 1.0;
  params.use_points_between_factor = false;
  params.position_covariance_threshold = 100;
  params.orientation_covariance_threshold = 100;
  params.point_to_point_error_threshold = 100;
  params.pose_translation_norm_threshold = 100;
  params.max_num_points_between_factors = 100;
  params.body_T_sensor = gtsam::Pose3::identity();
  params.enabled = true;
  return params;
}

TEST(CombinedNavStateNodeUpdaterTester, ConstantVelocity) {
  auto params = DefaultGraphLocalizerParams();
  // Use depth odometry factor adder since it can add relative pose factors
  params.factor.depth_odometry_adder = DefaultDepthOdometryFactorAdderParams();
  constexpr int kInitialVelocity = 0.1;
  params.graph_initializer.global_V_body_start = Eigen::Vector3d(kInitialVelocity, 0, 0);
  gl::GraphLocalizer graph_localizer(params);
  constexpr int kNumIterations = 100;
  constexpr double kTimeDiff = 0.1;
  lc::Time time = 0.0;
  const Eigen::Vector3d relative_translation = kTimeDiff * params.graph_initializer.global_V_body_start;
  // Don't need correspondences for this
  const std::vector<Eigen::Vector3d> zero_vector;
  const lm::DepthCorrespondences correspondences(zero_vector, zero_vector);
  for (int i = 0; i < kNumIterations; ++i) {
    time += kTimeDiff;
    const lm::ImuMeasurement zero_imu_measurement(Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), time);
    graph_localizer.AddImuMeasurement(zero_imu_measurement);
    const Eigen::Isometry3d relative_pose = lc::Isometry3d(relative_translation, Eigen::Matrix3d::Identity());
    const lc::PoseWithCovariance source_T_target(relative_pose, lc::PoseCovariance::Identity());
    lm::Odometry odometry;
    odometry.source_time = time - kTimeDiff;
    odometry.target_time = time;
    odometry.sensor_F_source_T_target = source_T_target;
    odometry.body_F_source_T_target = source_T_target;
    const lm::DepthOdometryMeasurement constant_velocity_measurement(odometry, correspondences, time);
    graph_localizer.AddDepthOdometryMeasurement(constant_velocity_measurement);
  }
  // TODO(rsoussan): check num factors in graph?
  // TODO(rsoussan): check each graph factor pose after optimization! make sure they are correct!!!
}

// Run all the tests that were declared with TEST()
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
