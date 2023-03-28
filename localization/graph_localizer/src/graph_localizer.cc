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

#include <graph_localizer/graph_localizer.h>

namespace graph_localizer {
namespace fa = factor_adders;
namespace lc = localization_common;
namespace lm = localization_measurements;
namespace na = node_adders;
namespace no = nodes;
namespace op = optimizers;

GraphLocalizer::GraphLocalizer(const GraphLocalizerParams& params)
    : SlidingWindowGraphOptimizer(params.sliding_window_graph_optimizer,
                                  std::make_unique<op::NonlinearOptimizer>(params.nonlinear_optimizer)),
      params_(params) {
  // Initialize node adders
  pose_node_adder_ =
    std::make_shared<na::PoseNodeAdder>(params_.pose_node_adder, params_.pose_node_adder_model, nodes());
  // Initialize factor adders
  loc_factor_adder_ =
    std::make_shared<fa::LocFactorAdder<na::PoseNodeAdder>>(params_.loc_factor_adder, pose_node_adder_);
}

void GraphLocalizer::AddPoseMeasurement(const lm::TimestampedPoseWithCovariance& pose_measurement) {
  pose_node_adder_->AddMeasurement(pose_measurement);
}

void GraphLocalizer::AddMatchedProjectionsMeasurement(
  const lm::MatchedProjectionsMeasurement& matched_projections_measurement) {
  loc_factor_adder_->AddMeasurement(matched_projections_measurement);
}

const no::TimestampedNodes<gtsam::Pose3>& GraphLocalizer::pose_nodes() const { return pose_node_adder_->nodes(); }
}  // namespace graph_localizer
