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

#ifndef TUTORIAL_EXAMPLES_RELATIVE_POSE_NODE_ADDER_MODEL_H_
#define TUTORIAL_EXAMPLES_RELATIVE_POSE_NODE_ADDER_MODEL_H_

#include <localization_common/pose_with_covariance_interpolater.h>
#include <localization_measurements/pose_with_covariance_measurement.h>
#include <node_adders/between_factor_node_adder_model.h>
#include <nodes/timestamped_nodes.h>

#include <gtsam/geometry/Pose3.h>
#include <gtsam/inference/Key.h>

#include <utility>

namespace tutorial_examples {
// Generates pose nodes and relative factors using interpolated
// pose measurements. Uses GTSAM pose between factors for
// relative factors.
class RelativePoseNodeAdderModel
    : public node_adders::
        BetweenFactorMeasurementBasedTimestampedNodeAdderModel<
          localization_measurements::
            PoseWithCovarianceMeasurement,
          gtsam::Pose3> {
 public:
  using Base = node_adders::
    BetweenFactorMeasurementBasedTimestampedNodeAdderModel<
      localization_measurements::PoseWithCovarianceMeasurement,
      gtsam::Pose3>;
  using NodesType = nodes::TimestampedNodes<gtsam::Pose3>;
  using Params = node_adders::TimestampedNodeAdderModelParams;

  explicit RelativePoseNodeAdderModel(const Params& params)
      : Base(params) {}

  // Adds a pose node at the provided timestamp using
  // the interpolated pose measurement at that timestamp.
  // TODO(rsoussan): This pose is in the wrong frame!!!!
  gtsam::KeyVector AddNode(
    const localization_common::Time timestamp,
    NodesType& nodes) const final {
    const auto pose = interpolator_.Interpolate(timestamp);
    // Add pose to nodes container to create a new node in
    // the graph.
    return nodes.Add(timestamp,
                     localization_common::GtPose(pose->pose));
  }

  // Adds a relative pose factor using the relative pose
  // between interpolated pose measurements at timestamp
  // a and b.
  boost::optional<
    std::pair<gtsam::Pose3, gtsam::SharedNoiseModel>>
  RelativeNodeAndNoise(
    const localization_common::Time timestamp_a,
    const localization_common::Time timestamp_b) const final {
    const auto relative_pose =
      interpolator_.Relative(timestamp_a, timestamp_b);
    const auto relative_pose_noise =
      gtsam::noiseModel::Gaussian::Covariance(
        relative_pose->covariance);
    return std::pair<gtsam::Pose3, gtsam::SharedNoiseModel>(
      localization_common::GtPose(relative_pose->pose),
      relative_pose_noise);
  }

  // Buffers a pose measurement for future use for node and
  // relative factor creation.
  void AddMeasurement(
    const localization_measurements::
      PoseWithCovarianceMeasurement& measurement) {
    interpolator_.Add(
      measurement.timestamp,
      localization_common::PoseWithCovariance(
        localization_common::EigenPose(measurement.pose),
        measurement.covariance));
  }

  // Removes old pose measurements.
  void RemoveMeasurements(
    const localization_common::Time oldest_allowed_time) {
    // Keep lower bound so future measurements can be
    // interpolated using it.
    interpolator_.RemoveBelowLowerBoundValues(
      oldest_allowed_time);
  }

  // Returns whether a node can be added at the provided
  // timestamp, which is determined by whether pose measurements
  // exist that bound that timestamp.
  bool CanAddNode(
    const localization_common::Time timestamp) const final {
    return interpolator_.WithinBounds(timestamp);
  }

 private:
  localization_common::PoseWithCovarianceInterpolater
    interpolator_;
};
}  // namespace tutorial_examples

#endif  // TUTORIAL_EXAMPLES_RELATIVE_POSE_NODE_ADDER_MODEL_H_
