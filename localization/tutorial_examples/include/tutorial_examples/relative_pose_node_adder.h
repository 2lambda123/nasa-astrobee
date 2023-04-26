/* Copyright (c) 2017, United States Government, as represented
 * by the Administrator of the National Aeronautics and Space
 * Administration.
 *
 * All rights reserved.
 *
 * The Astrobee platform is licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 */

#ifndef TUTORIAL_EXAMPLES_RELATIVE_POSE_NODE_ADDER_H_
#define TUTORIAL_EXAMPLES_RELATIVE_POSE_NODE_ADDER_H_

#include <localization_measurements/pose_with_covariance_measurement.h>
#include <node_adders/measurement_based_timestamped_node_adder.h>
#include <nodes/timestamped_nodes.h>
#include <tutorial_examples/relative_pose_node_adder_model.h>

#include <gtsam/geometry/Pose3.h>

namespace tutorial_examples {
using RelativePoseNodeAdderParams =
  node_adders::MeasurementBasedTimestampedNodeAdderParams<
    localization_measurements::PoseWithCovarianceMeasurement,
    gtsam::Pose3>;

using RelativePoseNodeAdder =
  node_adders::MeasurementBasedTimestampedNodeAdder<
    localization_measurements::PoseWithCovarianceMeasurement,
    gtsam::Pose3, nodes::TimestampedNodes<gtsam::Pose3>,
    RelativePoseNodeAdderModel>;
}  // namespace tutorial_examples

#endif  // TUTORIAL_EXAMPLES_RELATIVE_POSE_NODE_ADDER_H_
