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
#include <localization_common/test_utilities.h>
#include <localization_common/utilities.h>
#include <tutorial_examples/simple_localizer.h>

#include <gtest/gtest.h>

namespace lc = localization_common;
namespace lm = localization_measurements;
namespace te = tutorial_examples;

// TODO(rsoussan): add special clang format file?

lm::TimestampedPoseWithCovariance RandomPoseMeasurement(const lc::Time time) {
  return lm::TimestampedPoseWithCovariance(lc::RandomPoseWithCovariance(), time);
}

TEST(SimpleLocalizerTest, Interface) {
  te::SimpleLocalizerParams params;
  te::SimpleLocalizer localizer(params);
  // Add relative and absolute pose measurements at successive
  // timestamps
  for (int i = 0; i < 10; ++i) {
    localizer.AddRelativePoseMeasurement(RandomPoseMeasurement(i));
    localizer.AddAbsolutePoseMeasurement(RandomPoseMeasurement(i));
  }

  localizer.Update();

  // Access optimized timestamped nodes
  const auto& timestamped_nodes = localizer.timestamped_nodes();
  // Access optimized GTSAM values
  const auto& values = localizer.values();
  // Access GTSAM factors
  const auto& factors = localizer.factors();
  // Compute covariance for a node at timestamp 1
  const auto keys = timestamped_nodes.Keys(1);
  const auto covariance = localizer.Covariance(keys[0]);
}

// Run all the tests that were declared with TEST()
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
