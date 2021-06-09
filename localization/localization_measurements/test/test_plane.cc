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

#include <localization_common/logger.h>
#include <localization_measurements/plane.h>

#include <gtsam/base/numericalDerivative.h>

#include <gtest/gtest.h>

// gtsam::Point3 trans = Eigen::Vector3d::Random();
namespace lm = localization_measurements;

TEST(PlaneTester, PointToPlaneJacobian) {
  for (int i = 0; i < 500; ++i) {
    /*const gtsam::Point3 sensor_t_point = Eigen::Vector3d::Random();
    const gtsam::Pose3 world_T_line = RandomPose();
    const gtsam::Pose3 body_T_sensor = RandomPose();
    const gtsam::Pose3 world_T_body = RandomPose();
    const auto noise = gtsam::noiseModel::Unit::Create(2);
    const gtsam::PointToLineFactor factor(sensor_t_point, world_T_line, body_T_sensor, noise, sym::P(0));
    gtsam::Matrix H;
    const auto factor_error = factor.evaluateError(world_T_body, H);
    const auto numerical_H = gtsam::numericalDerivative11<gtsam::Vector, gtsam::Pose3>(
      boost::function<gtsam::Vector(const gtsam::Pose3&)>(
        boost::bind(&gtsam::PointToLineFactor::evaluateError, factor, _1, boost::none)),
      world_T_body, 1e-5);
    ASSERT_TRUE(numerical_H.isApprox(H.matrix(), 1e-6));*/
  }
}

TEST(PlaneTester, PointToPlaneDistanceXAxisPlane) {
  const gtsam::Point3 point(0, 0, 0);
  const gtsam::Vector3 normal(1, 0, 0);
  // Plane perpendicular to x-axis
  const lm::Plane x_axis_plane(point, normal);
  // In front of plane
  {
    const gtsam::Point3 test_point(1, 0, 0);
    const double distance = x_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, 1.0);
  }
  {
    const gtsam::Point3 test_point(1, 2, 3);
    const double distance = x_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, 1.0);
  }
  // Behind plane
  {
    const gtsam::Point3 test_point(-1, 0, 0);
    const double distance = x_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, -1.0);
  }
  {
    const gtsam::Point3 test_point(-1, 2, 3);
    const double distance = x_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, -1.0);
  }
  // On plane
  {
    const gtsam::Point3 test_point(0, 0, 0);
    const double distance = x_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, 0);
  }
  {
    const gtsam::Point3 test_point(0, 2, 3);
    const double distance = x_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, 0);
  }
}

TEST(PlaneTester, PointToPlaneDistanceYAxisPlane) {
  const gtsam::Point3 point(0, 1.1, 0);
  const gtsam::Vector3 normal(0, 1, 0);
  // Plane perpendicular to y-axis
  const lm::Plane y_axis_plane(point, normal);
  // In front of plane
  {
    const gtsam::Point3 test_point(0, 2.3, 0);
    const double distance = y_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, 1.2);
  }
  {
    const gtsam::Point3 test_point(1, 2.3, 3);
    const double distance = y_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, 1.2);
  }
  // Behind plane
  {
    const gtsam::Point3 test_point(-1, -7.9, 100);
    const double distance = y_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, -9);
  }
  {
    const gtsam::Point3 test_point(57, -7.9, 42.5);
    const double distance = y_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, -9);
  }
  // On plane
  {
    const gtsam::Point3 test_point(-3, 1.1, 100);
    const double distance = y_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, 0);
  }
  {
    const gtsam::Point3 test_point(21, 1.1, -3.3);
    const double distance = y_axis_plane.Distance(test_point);
    EXPECT_DOUBLE_EQ(distance, 0);
  }
}
