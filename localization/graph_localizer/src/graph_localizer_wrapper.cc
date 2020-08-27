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

#include <config_reader/config_reader.h>
#include <graph_localizer/graph_localizer_wrapper.h>
#include <graph_localizer/utilities.h>
#include <imu_integration/utilities.h>
#include <localization_common/utilities.h>
#include <localization_measurements/measurement_conversions.h>

#include <Eigen/Core>

#include <glog/logging.h>

namespace graph_localizer {
namespace lc = localization_common;
namespace lm = localization_measurements;
GraphLocalizerWrapper::GraphLocalizerWrapper() {
  // Needed for ConfigReader construction
  // TODO(rsoussan): load this somewhere else/ how do other nodelets do this?
  const std::string astrobee_configs_path = "/home/rsoussan/astrobee/astrobee";
  const std::string world = "iss";
  lc::SetEnvironmentConfigs(astrobee_configs_path, world);
  config_reader::ConfigReader config;

  config.AddFile("graph_localizer.config");
  config.AddFile("transforms.config");
  config.AddFile("cameras.config");
  config.AddFile("geometry.config");

  if (!config.ReadFiles()) {
    LOG(FATAL) << "Failed to read config files.";
  }

  if (!config.GetInt("num_bias_estimation_measurements", &num_bias_estimation_measurements_)) {
    LOG(FATAL) << "Failed to load num_bias_estimation_measurements.";
  }

  graph_loc_initialization_.LoadSensorParams(config);
  graph_loc_initialization_.LoadGraphLocalizerParams(config);
}

void GraphLocalizerWrapper::OpticalFlowCallback(const ff_msgs::Feature2dArray& feature_array_msg) {
  if (graph_localizer_)
    graph_localizer_->AddOpticalFlowMeasurement(lm::MakeFeaturePointsMeasurement(feature_array_msg));
}

void GraphLocalizerWrapper::ResetLocalizer() {
  LOG(INFO) << "ResetLocalizer: Resetting localizer.";
  graph_loc_initialization_.ResetStartPose();
  if (!have_latest_imu_biases_) {
    LOG(DFATAL) << "ResetLocalizer: Trying to reset localizer when no biases "
                   "are available.";
    return;
  }
  // TODO(rsoussan): compare current time with latest bias timestamp and print
  // warning if it is too old
  graph_loc_initialization_.SetBiases(latest_accelerometer_bias_, latest_gyro_bias_);
  graph_localizer_.reset();
}

void GraphLocalizerWrapper::ResetBiasesAndLocalizer() {
  LOG(INFO) << "ResetBiasAndLocalizer: Resetting biases and localizer.";
  graph_loc_initialization_.ResetBiasesAndStartPose();
  graph_localizer_.reset();
}

void GraphLocalizerWrapper::VLVisualLandmarksCallback(const ff_msgs::VisualLandmarks& visual_landmarks_msg) {
  if (graph_localizer_) {
    graph_localizer_->AddSparseMappingMeasurement(lm::MakeMatchedProjectionsMeasurement(visual_landmarks_msg));
  } else {
    // Set or update initial pose if a new one is available before the localizer
    // has started running.
    const Eigen::Isometry3d global_T_body =
        graph_localizer::EigenPose(visual_landmarks_msg, graph_loc_initialization_.params().body_T_nav_cam().inverse());
    const lc::Time timestamp = lc::TimeFromHeader(visual_landmarks_msg.header);
    graph_loc_initialization_.SetStartPose(global_T_body, timestamp);
  }
}

void GraphLocalizerWrapper::ARVisualLandmarksCallback(const ff_msgs::VisualLandmarks& visual_landmarks_msg) {
  if (graph_localizer_)
    graph_localizer_->AddARTagMeasurement(lm::MakeMatchedProjectionsMeasurement(visual_landmarks_msg));
}

void GraphLocalizerWrapper::ImuCallback(const sensor_msgs::Imu& imu_msg) {
  if (graph_localizer_) {
    graph_localizer_->AddImuMeasurement(lm::ImuMeasurement(imu_msg));
    const auto latest_biases = graph_localizer_->LatestBiases();
    if (!latest_biases) {
      LOG(WARNING) << "ImuCallback: Failed to get latest biases.";
    } else {
      latest_accelerometer_bias_ = latest_biases->first.accelerometer();
      latest_gyro_bias_ = latest_biases->first.gyroscope();
      latest_bias_timestamp_ = latest_biases->second;
      have_latest_imu_biases_ = true;
    }
  } else if (graph_loc_initialization_.EstimateBiases()) {
    EstimateAndSetImuBiases(lm::ImuMeasurement(imu_msg), num_bias_estimation_measurements_, imu_bias_measurements_,
                            graph_loc_initialization_);
  }

  // TODO(rsoussan): put this somewhere else?
  if (!graph_localizer_ && graph_loc_initialization_.ReadyToInitialize()) {
    InitializeGraph();
    LOG(INFO) << "ImuCallback: Initialized Graph.";
  }
}

void GraphLocalizerWrapper::InitializeGraph() {
  if (!graph_loc_initialization_.ReadyToInitialize()) {
    LOG(ERROR) << "InitializeGraph: Trying to initialize graph when not ready.";
    return;
  }

  graph_localizer_.reset(new graph_localizer::GraphLocalizer(graph_loc_initialization_.params()));
}

const FeatureTrackMap* const GraphLocalizerWrapper::feature_tracks() const {
  if (!graph_localizer_) return nullptr;
  return &(graph_localizer_->feature_tracks());
}

boost::optional<geometry_msgs::PoseStamped> GraphLocalizerWrapper::LatestPoseMsg() const {
  if (!graph_localizer_) {
    LOG(ERROR) << "LatestPoseMsg: Failed to get latest pose msg.";
    return boost::none;
  }
  return graph_localizer::LatestPoseMsg(*graph_localizer_);
}

boost::optional<ff_msgs::EkfState> GraphLocalizerWrapper::LatestLocalizationMsg() const {
  if (!graph_localizer_) {
    LOG_EVERY_N(WARNING, 50) << "LatestLocalizationMsg: Graph localizater not initialized yet.";
    return boost::none;
  }
  const auto combined_nav_state_and_covariances = graph_localizer_->LatestCombinedNavStateAndCovariances();
  if (!combined_nav_state_and_covariances) {
    LOG(ERROR) << "LatestLocalizationMsg: No combined nav state and covariances available.";
    return boost::none;
  }
  // Angular velocity and acceleration are added by imu integrator
  return EkfStateMsg(combined_nav_state_and_covariances->first, Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(),
                     combined_nav_state_and_covariances->second, graph_localizer_->NumOFFactors(),
                     graph_localizer_->NumVLFactors(), graph_loc_initialization_.EstimateBiases());
}
}  // namespace graph_localizer
