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

#include <ff_util/ff_names.h>
#include <graph_bag/depth_odometry_adder.h>
#include <graph_bag/utilities.h>
#include <localization_common/utilities.h>
#include <msg_conversions/msg_conversions.h>

#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <vector>

namespace graph_bag {
namespace lc = localization_common;
namespace lm = localization_measurements;
DepthOdometryAdder::DepthOdometryAdder(const std::string& input_bag_name, const std::string& output_bag_name)
    : input_bag_(input_bag_name, rosbag::bagmode::Read), output_bag_(output_bag_name, rosbag::bagmode::Write) {}

boost::optional<geometry_msgs::PoseWithCovarianceStamped> DepthOdometryAdder::GenerateDepthOdometry(
  const sensor_msgs::PointCloud2ConstPtr& depth_cloud_msg) {
  const lc::Time timestamp = lc::TimeFromHeader(depth_cloud_msg->header);
  std::pair<lc::Time, pcl::PointCloud<pcl::PointXYZ>::Ptr> depth_cloud{
    timestamp, pcl::PointCloud<pcl::PointXYZ>::Ptr(new pcl::PointCloud<pcl::PointXYZ>())};
  pcl::fromROSMsg(*depth_cloud_msg, *(depth_cloud.second));
  const auto relative_pose = depth_odometry_.DepthCloudCallback(depth_cloud);
  if (!relative_pose) {
    LogError("GenerateDepthOdometry: Failed to get relative pose.");
    return boost::none;
  }

  geometry_msgs::PoseWithCovarianceStamped pose_msg;
  msg_conversions::EigenPoseCovarianceToMsg(relative_pose->first, relative_pose->second, pose_msg);
  lc::TimeToHeader(depth_cloud.first, pose_msg.header);
  return pose_msg;
}

void DepthOdometryAdder::AddDepthOdometry() {
  const std::string depth_cloud_topic = static_cast<std::string>(TOPIC_HARDWARE_PICOFLEXX_PREFIX) +
                                        static_cast<std::string>(TOPIC_HARDWARE_NAME_HAZ_CAM) +
                                        static_cast<std::string>(TOPIC_HARDWARE_PICOFLEXX_SUFFIX);
  std::vector<std::string> topics;
  topics.push_back(depth_cloud_topic);
  topics.push_back(std::string("/") + depth_cloud_topic);
  topics.push_back(TOPIC_SPARSE_MAPPING_POSE);
  topics.push_back(std::string("/") + TOPIC_SPARSE_MAPPING_POSE);
  topics.push_back(TOPIC_GRAPH_LOC_STATE);
  topics.push_back(std::string("/") + TOPIC_GRAPH_LOC_STATE);
  topics.push_back(TOPIC_GNC_EKF);
  topics.push_back(std::string("/") + TOPIC_GNC_EKF);
  rosbag::View view(input_bag_, rosbag::TopicQuery(topics));
  for (const rosbag::MessageInstance msg : view) {
    if (string_ends_with(msg.getTopic(), depth_cloud_topic)) {
      const sensor_msgs::PointCloud2ConstPtr& depth_cloud_msg = msg.instantiate<sensor_msgs::PointCloud2>();
      const auto pose_msg = GenerateDepthOdometry(depth_cloud_msg);
      if (!pose_msg) continue;
      const ros::Time timestamp = lc::RosTimeFromHeader(depth_cloud_msg->header);
      output_bag_.write(std::string("/") + TOPIC_LOCALIZATION_DEPTH_ODOM, timestamp, *pose_msg);
    } else {
      output_bag_.write(msg.getTopic(), msg.getTime(), msg);
    }
  }
}
}  // namespace graph_bag
