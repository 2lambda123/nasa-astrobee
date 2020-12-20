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

#include <ff_common/utils.h>
#include <ff_util/ff_names.h>
#include <graph_bag/graph_bag.h>
#include <graph_bag/parameter_reader.h>
#include <graph_bag/utilities.h>
#include <graph_localizer/utilities.h>
#include <localization_common/utilities.h>
#include <msg_conversions/msg_conversions.h>

#include <geometry_msgs/PoseStamped.h>
#include <ros/time.h>
#include <sensor_msgs/Imu.h>

#include <Eigen/Core>

#include <glog/logging.h>

#include <chrono>
#include <cstdlib>
#include <vector>

namespace graph_bag {
namespace lc = localization_common;

GraphBag::GraphBag(const std::string& bag_name, const std::string& map_file, const std::string& image_topic,
                   const std::string& results_bag)
    : results_bag_(results_bag, rosbag::bagmode::Write) {
  config_reader::ConfigReader config;
  config.AddFile("cameras.config");
  config.AddFile("geometry.config");
  config.AddFile("tools/graph_bag.config");

  if (!config.ReadFiles()) {
    LOG(FATAL) << "Failed to read config files.";
  }

  LiveMeasurementSimulatorParams params;
  LoadLiveMeasurementSimulatorParams(config, bag_name, map_file, image_topic, params);
  live_measurement_simulator_.reset(new LiveMeasurementSimulator(params));

  GraphLocalizerSimulatorParams graph_params;
  LoadGraphLocalizerSimulatorParams(config, graph_params);
  graph_localizer_simulator_.reset(new GraphLocalizerSimulator(graph_params));

  save_optical_flow_images_ = params.save_optical_flow_images;
  // Needed for feature tracks visualization
  nav_cam_params_.reset(new camera::CameraParameters(&config, "nav_cam"));
  body_T_nav_cam_ = lc::LoadTransform(config, "nav_cam_transform");
}

void GraphBag::SavePoseMsg(const geometry_msgs::PoseStamped& pose_msg, const std::string& pose_topic) {
  const ros::Time timestamp = lc::RosTimeFromHeader(pose_msg.header);
  results_bag_.write("/" + pose_topic, timestamp, pose_msg);
}

void GraphBag::SaveOpticalFlowTracksImage(const sensor_msgs::ImageConstPtr& image_msg,
                                          const graph_localizer::FeatureTrackMap& feature_tracks) {
  const auto feature_track_image_msg = CreateFeatureTrackImage(image_msg, feature_tracks, *nav_cam_params_);
  if (!feature_track_image_msg) return;
  const ros::Time timestamp = lc::RosTimeFromHeader(image_msg->header);
  results_bag_.write("/" + kFeatureTracksImageTopic_, timestamp, **feature_track_image_msg);
}

void GraphBag::SaveImuBiasTesterPredictedStates(
  const std::vector<lc::CombinedNavState>& imu_bias_tester_predicted_states) {
  for (const auto state : imu_bias_tester_predicted_states) {
    geometry_msgs::PoseStamped pose_msg;
    lc::PoseToMsg(state.pose(), pose_msg.pose);
    lc::TimeToHeader(state.timestamp(), pose_msg.header);
    results_bag_.write("/" + kImuBiasTesterPoseTopic_ + "/pose", ros::Time(state.timestamp()), pose_msg);
  }
}

void GraphBag::SaveLocState(const ff_msgs::EkfState& loc_msg, const std::string& topic) {
  const ros::Time timestamp = lc::RosTimeFromHeader(loc_msg.header);
  results_bag_.write("/" + topic, timestamp, loc_msg);
}

void GraphBag::Run() {
  // Required to start bias estimation
  graph_localizer_simulator_->ResetBiasesAndLocalizer();
  const auto start_time = std::chrono::steady_clock::now();
  while (live_measurement_simulator_->ProcessMessage()) {
    const lc::Time current_time = live_measurement_simulator_->CurrentTime();
    const auto imu_msg = live_measurement_simulator_->GetImuMessage(current_time);
    if (imu_msg) {
      graph_localizer_simulator_->BufferImuMsg(*imu_msg);
      imu_augmentor_wrapper_.ImuCallback(*imu_msg);
      imu_bias_tester_wrapper_.ImuCallback(*imu_msg);

      // Save imu augmented loc msg if available
      const auto imu_augmented_loc_msg = imu_augmentor_wrapper_.LatestImuAugmentedLocalizationMsg();
      if (!imu_augmented_loc_msg) {
        LOG_EVERY_N(WARNING, 50) << "Run: Failed to get latest imu augmented loc msg.";
      } else {
        SaveLocState(*imu_augmented_loc_msg, TOPIC_GNC_EKF);
      }
    }
    const auto of_msg = live_measurement_simulator_->GetOFMessage(current_time);
    if (of_msg) {
      graph_localizer_simulator_->BufferOpticalFlowMsg(*of_msg);
      if (save_optical_flow_images_) {
        const auto img_msg = live_measurement_simulator_->GetImageMessage(lc::TimeFromHeader(of_msg->header));
        if (img_msg && graph_localizer_simulator_->feature_tracks())
          SaveOpticalFlowTracksImage(*img_msg, *(graph_localizer_simulator_->feature_tracks()));
      }
    }
    const auto vl_msg = live_measurement_simulator_->GetVLMessage(current_time);
    if (vl_msg) {
      graph_localizer_simulator_->BufferVLVisualLandmarksMsg(*vl_msg);
      if (vl_msg->landmarks.size() >= 5) {
        const gtsam::Pose3 sparse_mapping_global_T_body = lc::GtPose(*vl_msg, body_T_nav_cam_.inverse());
        const lc::Time timestamp = lc::TimeFromHeader(vl_msg->header);
        SavePoseMsg(graph_localizer::PoseMsg(sparse_mapping_global_T_body, timestamp), TOPIC_SPARSE_MAPPING_POSE);
      }
    }
    const auto ar_msg = live_measurement_simulator_->GetARMessage(current_time);
    if (ar_msg) {
      graph_localizer_simulator_->BufferARVisualLandmarksMsg(*ar_msg);
      if (ar_msg->landmarks.size() >= 4) {
        const auto ar_tag_pose_msg = graph_localizer_simulator_->LatestARTagPoseMsg();
        if (!ar_tag_pose_msg) {
          LogWarning("Run: Failed to get ar tag pose msg");
        } else {
          static lc::Time last_added_timestamp = 0;
          const auto timestamp = lc::TimeFromHeader(ar_tag_pose_msg->header);
          // Prevent adding the same pose twice, since the pose is buffered before adding to the graph localizer
          // wrapper in the graph localizer simulator and LatestARTagPoseMsg returns
          // the last pose that has already been added to the graph localizer wrapper.
          if (last_added_timestamp != timestamp) {
            SavePoseMsg(*ar_tag_pose_msg, TOPIC_AR_TAG_POSE);
            last_added_timestamp = timestamp;
          }
        }
      }
    }

    const bool updated_graph = graph_localizer_simulator_->AddMeasurementsAndUpdateIfReady(current_time);
    if (updated_graph) {
      // Save latest graph localization msg
      // Pass latest loc state to imu augmentor if it is available.
      const auto localization_msg = graph_localizer_simulator_->LatestLocalizationStateMsg();
      if (!localization_msg) {
        LOG_EVERY_N(WARNING, 50) << "Run: Failed to get localization msg.";
      } else {
        imu_augmentor_wrapper_.LocalizationStateCallback(*localization_msg);
        SaveLocState(*localization_msg, TOPIC_GRAPH_LOC_STATE);
        const auto imu_bias_tester_predicted_states =
          imu_bias_tester_wrapper_.LocalizationStateCallback(*localization_msg);
        SaveImuBiasTesterPredictedStates(imu_bias_tester_predicted_states);
      }
    }
  }
  const auto end_time = std::chrono::steady_clock::now();
  LOG(INFO) << "Total run time: " << std::chrono::duration<double>(end_time - start_time).count() << " seconds.";
}
}  // namespace graph_bag
