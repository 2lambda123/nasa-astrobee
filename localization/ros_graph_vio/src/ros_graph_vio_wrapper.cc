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
#include <graph_vio/graph_vio_wrapper.h>
#include <localization_common/logger.h>
#include <localization_common/utilities.h>
#include <localization_measurements/measurement_conversions.h>
#include <msg_conversions/msg_conversions.h>

#include <Eigen/Core>

namespace graph_vio {
namespace lc = localization_common;
namespace lm = localization_measurements;
namespace mc = msg_conversions;

GraphVIOWrapper::GraphVIOWrapper(const std::string& graph_config_path_prefix) { LoadConfigs(graph_config_path_prefix); }

void GraphVIOWrapper::LoadConfigs(const std::string& graph_config_path_prefix) {
  config_reader::ConfigReader config;
  lc::LoadGraphVIOConfig(config, graph_config_path_prefix);
  config.AddFile("transforms.config");
  config.AddFile("cameras.config");
  config.AddFile("geometry.config");

  if (!config.ReadFiles()) {
    LogFatal("Failed to read config files.");
  }

  if (!config.GetBool("publish_graph", &publish_graph_)) {
    LogFatal("Failed to load publish_graph.");
  }

  if (!config.GetBool("save_graph_dot_file", &save_graph_dot_file_)) {
    LogFatal("Failed to load save_graph_dot_file.");
  }

  graph_vio::LoadGraphVIOParams(config, params_);
}

void GraphVIOWrapper::FeaturePointsCallback(const ff_msgs::Feature2dArray& feature_array_msg) {
  if (Initialized()) graph_vio_->AddOpticalFlowMeasurement(lm::MakeFeaturePointsMeasurement(feature_array_msg));
}

void GraphVIOWrapper::ImuCallback(const sensor_msgs::Imu& imu_msg) {
  if (Initialized()) {
    graph_vio_->AddImuMeasurement(lm::ImuMeasurement(imu_msg));
    /*const auto latest_biases = graph_vio_->LatestBiases();
    if (!latest_biases) {
      LogError("ImuCallback: Failed to get latest biases.");
    } else {
      latest_biases_ = latest_biases->first;
    }*/
  } else if (imu_bias_initializer_.EstimateBiases()) {
    imu_bias_initializer_.EstimateAndSetImuBiases(lm::ImuMeasurement(imu_msg), fan_speed_mode_);
    if (imu_bias_initializer_.HasBiases())
      latest_biases_ = imu_bias_initializer_.params().graph_initializer.initial_imu_bias;
  }

  if (!Initialized() && imu_bias_initializer_.ReadyToInitialize()) {
    InitializeGraph();
    LogDebug("ImuCallback: Initialized Graph.");
  }
}

void GraphVIOWrapper::FlightModeCallback(const ff_msgs::FlightMode& flight_mode) {
  fan_speed_mode_ = lm::ConvertFanSpeedMode(flight_mode.speed);
  // if (graph_vio_) graph_vio_->SetFanSpeedMode(fan_speed_mode_);
  imu_bias_initializer_.SetFanSpeedMode(fan_speed_mode_);
}

void GraphVIOWrapper::Update() {
  if (Initialized()) graph_vio_->Update();
}

void GraphVIOWrapper::Initialize() {
  if (!imu_bias_initializer_.ReadyToInitialize()) {
    LogDebug("InitializeGraph: Trying to initialize graph when not ready.");
    return;
  }

  graph_vio_.reset(new graph_vio::GraphVIO(imu_bias_initializer_.params()));
}

bool GraphVIOWrapper::Initialized() const { return graph_vio_; }

boost::optional<lc::CombinedNavState> GraphVIOWrapper::LatestCombinedNavState() const {
  if (!graph_vio_) {
    LogWarningEveryN(50, "LatestCombinedNavState: Graph localizater not initialized yet.");
    return boost::none;
  }
  const auto latest_combined_nav_state = graph_vio_->LatestCombinedNavState();
  if (!latest_combined_nav_state) {
    LogWarningEveryN(50, "LatestCombinedNavState: No combined nav state available.");
    return boost::none;
  }
  return latest_combined_nav_state;
}

boost::optional<ff_msgs::GraphVIOState> GraphVIOWrapper::LatestVIOStateMsg() {
  if (!graph_vio_) {
    LogDebugEveryN(50, "LatestVIOMsg: Graph VIO not initialized yet.");
    return boost::none;
  }
  const auto combined_nav_state_and_covariances = graph_vio_->LatestCombinedNavStateAndCovariances();
  if (!combined_nav_state_and_covariances) {
    LogDebugEveryN(50, "LatestVIOMsg: No combined nav state and covariances available.");
    return boost::none;
  }
  const auto graph_state_msg =
    GraphStateMsg(combined_nav_state_and_covariances->first, combined_nav_state_and_covariances->second,
                  feature_counts_, imu_bias_initializer_.EstimateBiases(), position_cov_log_det_lost_threshold_,
                  orientation_cov_log_det_lost_threshold_, graph_vio_->standstill(), graph_vio_->graph_vio_stats(),
                  graph_vio_->fan_speed_mode());
  feature_counts_.Reset();
  return graph_state_msg;
}

boost::optional<ff_msgs::SerializedGraph> GraphVIOWrapper::LatestGraphMsg() const {
  if (!graph_vio_) {
    LogWarningEveryN(50, "LatestVIOGraphMsg: Graph VIO not initialized yet.");
    return boost::none;
  }
  return GraphMsg(*graph_vio_);
}

void GraphVIOWrapper::ResetVIO() {
  LogInfo("ResetVIO: Resetting vio.");
  if (!latest_biases_) {
    LogError(
      "ResetVIO: Trying to reset vio when no biases "
      "are available.");
    return;
  }

  // TODO(rsoussan): compare current time with latest bias timestamp and print
  // warning if it is too old
  imu_bias_initializer_.SetBiases(*latest_biases_, true);
  graph_vio_.reset();
}

void GraphVIOWrapper::ResetBiasesAndVIO() {
  LogInfo("ResetBiasAndVIO: Resetting biases and vio.");
  imu_bias_initializer_.ResetBiases();
  graph_vio_.reset();
}

void GraphVIOWrapper::ResetBiasesFromFileAndResetVIO() {
  LogInfo("ResetBiasAndVIO: Resetting biases from file and resetting vio.");
  imu_bias_initializer_.ResetBiasesFromFile();
  if (imu_bias_initializer_.HasBiases())
    latest_biases_ = imu_bias_initializer_.params().graph_initializer.initial_imu_bias;
  graph_vio_.reset();
}

void GraphVIOWrapper::SaveGraphDotFile() const {
  if (graph_vio_) graph_vio_->SaveGraphDotFile();
}

/*boost::optional<const GraphVIOStats&> GraphVIOWrapper::graph_stats() const {
  if (!graph_vio_) {
    LogDebug("GraphStats: Failed to get graph stats.");
    return boost::none;
  }
  return graph_vio_->graph_vio_stats();
}*/

bool GraphVIOWrapper::publish_graph() const { return publish_graph_; }

bool GraphVIOWrapper::save_graph_dot_file() const { return save_graph_dot_file_; }

}  // namespace graph_vio
