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

#include <localization_analysis/feature_track_image_adder.h>

#include <cv_bridge/cv_bridge.h>

namespace localization_analysis {
namespace vc = vision_common;

void FeatureTrackImage(const vc::SpacedFeatureTracker& feature_tracker,
                       const camera::CameraParameters& camera_params, cv::Mat& feature_track_image) {
  const auto spaced_feature_tracks = feature_tracker.SpacedFeatureTracks();
  for (const auto& feature_track : spaced_feature_tracks) {
    cv::Scalar color;
    if (feature_track.size() <= 1) {
      // Red for single point tracks
      color = cv::Scalar(50, 255, 50, 1);
    } else if (feature_track.size() < 3) {
      // Yellow for medium length tracks
      color = cv::Scalar(255, 255, 0, 1);
    } else {
      // Green for long tracks
      color = cv::Scalar(50, 255, 50, 1);
    }

    // Draw track history
    if (feature_track.size() > 1) {
      for (auto point_it = feature_track.begin(); point_it != std::prev(feature_track.end()); ++point_it) {
        const auto& point1 = point_it->image_point;
        const auto& point2 = std::next(point_it)->image_point;
        const auto distorted_previous_point = Distort(point1, camera_params);
        const auto distorted_current_point = Distort(point2, camera_params);
        cv::circle(feature_track_image, distorted_current_point, 2 /* Radius*/, cv::Scalar(0, 255, 255), -1 /*Filled*/,
                   8);
        cv::line(feature_track_image, distorted_current_point, distorted_previous_point, color, 2, 8, 0);
      }
    } else {
      cv::circle(feature_track_image, Distort(feature_track.cbegin()->image_point, camera_params), 2 /* Radius*/, color,
                 -1 /*Filled*/, 8);
    }
    // Draw feature id at most recent point
    cv::putText(feature_track_image, std::to_string(feature_track.crbegin()->feature_track_id),
                Distort(feature_track.crbegin()->image_point, camera_params), cv::FONT_HERSHEY_PLAIN, 0.4,
                cv::Scalar(255, 0, 0));
  }
}

void MarkSmartFactorPoints(const std::vector<boost::shared_ptr<const factor_adders::RobustSmartFactor>> smart_factors,
                           const camera::CameraParameters& camera_params, cv::Mat& feature_track_image) {
  for (const auto& smart_factor : smart_factors) {
    const auto& point = smart_factor->measured().back();
    const auto distorted_point = Distort(point, camera_params);
    cv::circle(feature_track_image, distorted_point, 15 /* Radius*/, cv::Scalar(200, 100, 0), -1 /*Filled*/, 8);
  }
}

boost::optional<sensor_msgs::ImagePtr> CreateFeatureTrackImage(
  const sensor_msgs::ImageConstPtr& image_msg, const vc::SpacedFeatureTracker& feature_tracker,
  const camera::CameraParameters& camera_params,
  const std::vector<boost::shared_ptr<const factor_adders::RobustSmartFactor>>& smart_factors) {
  cv_bridge::CvImagePtr feature_track_image;
  try {
    feature_track_image = cv_bridge::toCvCopy(image_msg, sensor_msgs::image_encodings::RGB8);
  } catch (cv_bridge::Exception& e) {
    ROS_ERROR("cv_bridge exception: %s", e.what());
    return boost::none;
  }

  FeatureTrackImage(feature_tracker, camera_params, feature_track_image->image);
  MarkSmartFactorPoints(smart_factors, camera_params, feature_track_image->image);
  return feature_track_image->toImageMsg();
}

cv::Point2f Distort(const Eigen::Vector2d& undistorted_point, const camera::CameraParameters& params) {
  Eigen::Vector2d distorted_point;
  params.Convert<camera::UNDISTORTED_C, camera::DISTORTED>(undistorted_point, &distorted_point);
  return cv::Point2f(distorted_point.x(), distorted_point.y());
}
}  // namespace localization_analysis
