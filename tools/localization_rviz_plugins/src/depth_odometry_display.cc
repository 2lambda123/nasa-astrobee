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
#include <QObject>
#include <ff_util/ff_names.h>
#include <localization_common/logger.h>
#include <localization_common/utilities.h>
#include <localization_measurements/measurement_conversions.h>
#include <msg_conversions/msg_conversions.h>

#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>

#include <cv_bridge/cv_bridge.h>
#include <opencv2/calib3d.hpp>
#include <pcl/common/transforms.h>
#include <pcl_conversions/pcl_conversions.h>
#include <rviz/frame_manager.h>
#include <rviz/visualization_manager.h>

#include "depth_odometry_display.h"  // NOLINT
#include "utilities.h"               // NOLINT

namespace localization_rviz_plugins {
namespace lc = localization_common;
namespace lm = localization_measurements;

DepthOdometryDisplay::DepthOdometryDisplay() {
  int ff_argc = 1;
  char argv[] = "depth_odometry_display";
  char* argv_ptr = &argv[0];
  char** argv_ptr_ptr = &argv_ptr;
  ff_common::InitFreeFlyerApplication(&ff_argc, &argv_ptr_ptr);
  config_reader::ConfigReader config;
  config.AddFile("cameras.config");
  config.AddFile("geometry.config");
  config.AddFile("transforms.config");
  if (!config.ReadFiles()) {
    LogFatal("Failed to read config files.");
  }
  camera_params_.reset(new camera::CameraParameters(&config, "haz_cam"));
  const auto focal_lengths = camera_params_->GetFocalVector();
  const auto distortion_params = camera_params_->GetDistortion();
  const auto principal_points = camera_params_->GetOpticalOffset();
  intrinsics_ = cv::Mat::zeros(3, 3, cv::DataType<double>::type);
  intrinsics_.at<double>(0, 0) = focal_lengths[0];
  intrinsics_.at<double>(1, 1) = focal_lengths[1];
  intrinsics_.at<double>(0, 2) = principal_points[0];
  intrinsics_.at<double>(1, 2) = principal_points[1];
  intrinsics_.at<double>(2, 2) = 1;
  distortion_params_ = cv::Mat(4, 1, cv::DataType<double>::type);
  for (int i = 0; i < 4; ++i) {
    distortion_params_.at<double>(i, 0) = distortion_params[i];
  }

  correspondence_index_slider_.reset(new rviz::SliderProperty("Select Correspondence", 0, "Select Correspondence.",
                                                              this, SLOT(createCorrespondencesImage())));

  image_transport::ImageTransport image_transport(nh_);
  const std::string image_topic = /*static_cast<std::string>(TOPIC_HARDWARE_PICOFLEXX_PREFIX) +
                                  static_cast<std::string>(TOPIC_HARDWARE_NAME_HAZ_CAM) +
                                  static_cast<std::string>(TOPIC_HARDWARE_PICOFLEXX_SUFFIX_DEPTH_IMAGE);*/
    "/hw/depth_haz/extended/amplitude_int";
  image_sub_ = image_transport.subscribe(image_topic, 10, &DepthOdometryDisplay::imageCallback, this);
  const std::string point_cloud_topic = static_cast<std::string>(TOPIC_HARDWARE_PICOFLEXX_PREFIX) +
                                        static_cast<std::string>(TOPIC_HARDWARE_NAME_HAZ_CAM) +
                                        static_cast<std::string>(TOPIC_HARDWARE_PICOFLEXX_SUFFIX);
  point_cloud_sub_ = nh_.subscribe<sensor_msgs::PointCloud2>(
    point_cloud_topic, 10, &DepthOdometryDisplay::pointCloudCallback, this, ros::TransportHints().tcpNoDelay());
  depth_odom_sub_ = nh_.subscribe<geometry_msgs::PoseWithCovarianceStamped>(TOPIC_LOCALIZATION_DEPTH_ODOM, 10,
                                                                            &DepthOdometryDisplay::depthOdomCallback,
                                                                            this, ros::TransportHints().tcpNoDelay());
  correspondence_image_pub_ = image_transport.advertise("/depth_odom/correspondence_image", 1);
  projection_image_pub_ = image_transport.advertise("/depth_odom/projection_image", 1);
  source_correspondence_point_pub_ = nh_.advertise<geometry_msgs::PointStamped>("source_point_with_correspondence", 10);
  target_correspondence_point_pub_ = nh_.advertise<geometry_msgs::PointStamped>("target_point_with_correspondence", 10);
  source_point_cloud_pub_ = nh_.advertise<sensor_msgs::PointCloud2>("source_cloud", 10);
  target_point_cloud_pub_ = nh_.advertise<sensor_msgs::PointCloud2>("target_cloud", 10);
}

void DepthOdometryDisplay::onInitialize() { MFDClass::onInitialize(); }

void DepthOdometryDisplay::reset() {
  MFDClass::reset();
  clearDisplay();
}

void DepthOdometryDisplay::clearDisplay() {}

void DepthOdometryDisplay::imageCallback(const sensor_msgs::ImageConstPtr& image_msg) {
  img_buffer_.Add(lc::TimeFromHeader(image_msg->header), image_msg);
}

void DepthOdometryDisplay::pointCloudCallback(const sensor_msgs::PointCloud2ConstPtr& point_cloud_msg) {
  pcl::PointCloud<pcl::PointXYZ>::Ptr point_cloud(new pcl::PointCloud<pcl::PointXYZ>());
  pcl::fromROSMsg(*point_cloud_msg, *point_cloud);
  point_cloud_buffer_.Add(lc::TimeFromHeader(point_cloud_msg->header), point_cloud);
}

void DepthOdometryDisplay::depthOdomCallback(const geometry_msgs::PoseWithCovarianceStampedConstPtr& depth_odom_msg) {
  // TODO(rsoussan): Add better conversion from pose_msg to isometry3d
  const auto pose = msg_conversions::ros_pose_to_eigen_transform(depth_odom_msg->pose.pose);
  const auto time = lc::TimeFromHeader(depth_odom_msg->header);
  relative_pose_buffer_.Add(time, pose);
}

void DepthOdometryDisplay::processMessage(const ff_msgs::DepthImageCorrespondences::ConstPtr& correspondences_msg) {
  latest_correspondences_msg_ = correspondences_msg;
  createCorrespondencesImage();
  createProjectionImage();
}

cv::Point2f DepthOdometryDisplay::projectPoint(const pcl::PointXYZ& point_3d) {
  cv::Mat zero_r(cv::Mat::eye(3, 3, cv::DataType<double>::type));
  cv::Mat zero_t(cv::Mat::zeros(3, 1, cv::DataType<double>::type));
  std::vector<cv::Point2d> projected_points;
  std::vector<cv::Point3d> object_points;
  object_points.emplace_back(cv::Point3d(point_3d.x, point_3d.y, point_3d.z));
  cv::projectPoints(object_points, zero_r, zero_t, intrinsics_, distortion_params_, projected_points);
  return projected_points[0];
}

void DepthOdometryDisplay::createProjectionImage() {
  if (!latest_correspondences_msg_) return;
  const lc::Time source_time = lc::TimeFromRosTime(latest_correspondences_msg_->source_time);
  const lc::Time target_time = lc::TimeFromRosTime(latest_correspondences_msg_->target_time);
  const auto source_image_msg = img_buffer_.Get(source_time);
  const auto target_image_msg = img_buffer_.Get(target_time);
  if (!source_image_msg || !target_image_msg) return;

  const auto relative_pose = relative_pose_buffer_.Get(target_time);
  if (!relative_pose) return;

  const auto source_point_cloud = point_cloud_buffer_.GetNearby(source_time, 0.05);
  if (!source_point_cloud) return;

  pcl::PointCloud<pcl::PointXYZ>::Ptr transformed_point_cloud(new pcl::PointCloud<pcl::PointXYZ>);
  pcl::transformPointCloud(**source_point_cloud, *transformed_point_cloud, *relative_pose);

  auto source_image_measurement = lm::MakeImageMeasurement(*source_image_msg, sensor_msgs::image_encodings::RGB8);
  if (!source_image_measurement) {
    return;
  }
  auto& source_image = source_image_measurement->image;

  auto target_image_measurement = lm::MakeImageMeasurement(*target_image_msg, sensor_msgs::image_encodings::RGB8);
  if (!target_image_measurement) {
    return;
  }
  auto& target_image = target_image_measurement->image;

  // Draw optical flow tracks in target image, add projected 3D points as well
  cv_bridge::CvImage projection_image;
  projection_image.encoding = sensor_msgs::image_encodings::RGB8;
  const int rows = target_image.rows;
  const int cols = target_image.cols;
  projection_image.image = cv::Mat(rows * 2, cols, CV_8UC3, cv::Scalar(0, 0, 0));

  for (const auto& correspondence : latest_correspondences_msg_->correspondences) {
    const int source_correspondence_index = correspondence.source_index;
    const int source_correspondence_row = source_correspondence_index / cols;
    const int source_correspondence_col = source_correspondence_index - cols * source_correspondence_row;
    const cv::Point source_correspondence_image_point(source_correspondence_col, source_correspondence_row);
    const auto projected_source_point = projectPoint(transformed_point_cloud->points[source_correspondence_index]);
    const int target_correspondence_index = correspondence.target_index;
    const int target_correspondence_row = target_correspondence_index / cols;
    const int target_correspondence_col = target_correspondence_index - cols * target_correspondence_row;
    const cv::Point target_correspondence_image_point(target_correspondence_col, target_correspondence_row);
    cv::circle(source_image, source_correspondence_image_point, 1 /* Radius*/, cv::Scalar(0, 255, 0), -1 /*Filled*/, 8);
    cv::circle(target_image, target_correspondence_image_point, 1 /* Radius*/, cv::Scalar(0, 255, 0), -1 /*Filled*/, 8);
    cv::circle(target_image, projected_source_point, 1 /* Radius*/, cv::Scalar(255, 0, 0), -1 /*Filled*/, 8);
    cv::line(target_image, target_correspondence_image_point, source_correspondence_image_point, cv::Scalar(0, 255, 0),
             2, 8, 0);
    cv::line(target_image, target_correspondence_image_point, projected_source_point, cv::Scalar(255, 0, 0), 2, 8, 0);
  }
  source_image.copyTo(projection_image.image(cv::Rect(0, 0, cols, rows)));
  target_image.copyTo(projection_image.image(cv::Rect(0, rows, cols, rows)));
  projection_image_pub_.publish(projection_image.toImageMsg());
}

void DepthOdometryDisplay::createCorrespondencesImage() {
  clearDisplay();
  if (!latest_correspondences_msg_) return;
  const lc::Time source_time = lc::TimeFromRosTime(latest_correspondences_msg_->source_time);
  const lc::Time target_time = lc::TimeFromRosTime(latest_correspondences_msg_->target_time);
  const auto source_image_msg = img_buffer_.Get(source_time);
  const auto target_image_msg = img_buffer_.Get(target_time);
  if (!source_image_msg || !target_image_msg) return;

  // TODO(rsoussan): make function for this, unify with loc graph display
  cv_bridge::CvImagePtr source_cv_image;
  try {
    source_cv_image = cv_bridge::toCvCopy(*source_image_msg, sensor_msgs::image_encodings::RGB8);
  } catch (cv_bridge::Exception& e) {
    LogError("cv_bridge exception: " << e.what());
    return;
  }
  auto& source_image = source_cv_image->image;

  cv_bridge::CvImagePtr target_cv_image;
  try {
    target_cv_image = cv_bridge::toCvCopy(*target_image_msg, sensor_msgs::image_encodings::RGB8);
  } catch (cv_bridge::Exception& e) {
    LogError("cv_bridge exception: " << e.what());
    return;
  }
  auto& target_image = target_cv_image->image;

  // Create correspondence image
  // Draw source image above target image, add correspondences as points outlined
  // by rectangles to each image
  cv_bridge::CvImage correspondence_image;
  correspondence_image.encoding = sensor_msgs::image_encodings::RGB8;
  const int rows = target_image.rows;
  const int cols = target_image.cols;
  correspondence_image.image = cv::Mat(rows * 2, cols, CV_8UC3, cv::Scalar(0, 0, 0));

  correspondence_index_slider_->setMaximum(latest_correspondences_msg_->correspondences.size() - 1);
  const int correspondence_index = correspondence_index_slider_->getInt();
  const auto correspondence = latest_correspondences_msg_->correspondences[correspondence_index];
  const int source_correspondence_index = correspondence.source_index;
  const int source_correspondence_row = source_correspondence_index / cols;
  const int source_correspondence_col = source_correspondence_index - cols * source_correspondence_row;
  const cv::Point source_correspondence_image_point(source_correspondence_col, source_correspondence_row);
  const int target_correspondence_index = correspondence.target_index;
  const int target_correspondence_row = target_correspondence_index / cols;
  const int target_correspondence_col = target_correspondence_index - cols * target_correspondence_row;
  const cv::Point target_correspondence_image_point(target_correspondence_col, target_correspondence_row);
  const cv::Point rectangle_offset(40, 40);
  cv::circle(source_image, source_correspondence_image_point, 5 /* Radius*/, cv::Scalar(0, 255, 0), -1 /*Filled*/, 8);
  cv::rectangle(source_image, source_correspondence_image_point - rectangle_offset,
                source_correspondence_image_point + rectangle_offset, cv::Scalar(0, 255, 0), 8);
  cv::circle(target_image, target_correspondence_image_point, 5 /* Radius*/, cv::Scalar(0, 255, 0), -1 /*Filled*/, 8);
  cv::rectangle(target_image, target_correspondence_image_point - rectangle_offset,
                target_correspondence_image_point + rectangle_offset, cv::Scalar(0, 255, 0), 8);
  source_image.copyTo(correspondence_image.image(cv::Rect(0, 0, cols, rows)));
  target_image.copyTo(correspondence_image.image(cv::Rect(0, rows, cols, rows)));
  correspondence_image_pub_.publish(correspondence_image.toImageMsg());
  publishCorrespondencePoints(correspondence, source_time, target_time);
}

void DepthOdometryDisplay::publishCorrespondencePoints(const ff_msgs::DepthImageCorrespondence& correspondence,
                                                       const lc::Time source_time, const lc::Time target_time) {
  const auto source_point_cloud = point_cloud_buffer_.GetNearby(source_time, 0.05);
  const auto target_point_cloud = point_cloud_buffer_.GetNearby(target_time, 0.05);
  if (!source_point_cloud || !target_point_cloud) return;
  geometry_msgs::PointStamped source_correspondence_point_msg;
  const auto source_correspondence_point = (*source_point_cloud)->points[correspondence.source_index];
  source_correspondence_point_msg.point.x = source_correspondence_point.x;
  source_correspondence_point_msg.point.y = source_correspondence_point.y;
  source_correspondence_point_msg.point.z = source_correspondence_point.z;
  source_correspondence_point_msg.header.stamp = ros::Time::now();
  source_correspondence_point_msg.header.frame_id = "haz_cam";
  source_correspondence_point_pub_.publish(source_correspondence_point_msg);

  geometry_msgs::PointStamped target_correspondence_point_msg;
  const auto target_correspondence_point = (*target_point_cloud)->points[correspondence.target_index];
  target_correspondence_point_msg.point.x = target_correspondence_point.x;
  target_correspondence_point_msg.point.y = target_correspondence_point.y;
  target_correspondence_point_msg.point.z = target_correspondence_point.z;
  target_correspondence_point_msg.header.stamp = ros::Time::now();
  target_correspondence_point_msg.header.frame_id = "haz_cam";
  target_correspondence_point_pub_.publish(target_correspondence_point_msg);

  {
    const auto source_cloud_msg =
      lm::MakePointCloudMsg(**source_point_cloud, lc::TimeFromRosTime(ros::Time::now()), "haz_cam");
    source_point_cloud_pub_.publish(source_cloud_msg);
    const auto target_cloud_msg =
      lm::MakePointCloudMsg(**target_point_cloud, lc::TimeFromRosTime(ros::Time::now()), "haz_cam");
    target_point_cloud_pub_.publish(target_cloud_msg);
  }

  point_cloud_buffer_.EraseUpTo(source_time);
  img_buffer_.EraseUpTo(source_time);
}
}  // namespace localization_rviz_plugins

#include <pluginlib/class_list_macros.h>  // NOLINT
PLUGINLIB_EXPORT_CLASS(localization_rviz_plugins::DepthOdometryDisplay, rviz::Display)
