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
#ifndef GRAPH_LOCALIZER_GRAPH_LOCALIZER_STATS_H_
#define GRAPH_LOCALIZER_GRAPH_LOCALIZER_STATS_H_

#include <graph_optimizer/graph_stats.h>

namespace graph_localizer {
class GraphLocalizerStats : public graph_optimizer::GraphStats {
 public:
  GraphLocalizerStats();
  void UpdateErrors(const gtsam::NonlinearFactorGraph& graph_factors,
                    const graph_optimizer::GraphValues& graph_values) final;
  void UpdateSpecificStats(const gtsam::NonlinearFactorGraph& graph_factors,
                           const graph_optimizer::GraphValues& graph_values) final;

  // Graph Stats Averagers
  localization_common::Averager num_optical_flow_factors_averager_ =
    localization_common::Averager("Num Optical Flow Factors");
  localization_common::Averager num_loc_proj_factors_averager_ = localization_common::Averager("Num Loc Proj Factors");
  localization_common::Averager num_loc_pose_factors_averager_ = localization_common::Averager("Num Loc Pose Factors");
  localization_common::Averager num_imu_factors_averager_ = localization_common::Averager("Num Imu Factors");
  localization_common::Averager num_rotation_factors_averager_ = localization_common::Averager("Num Rotation Factors");
  localization_common::Averager num_standstill_between_factors_averager_ =
    localization_common::Averager("Num Standstill Between Factors");
  localization_common::Averager num_vel_prior_factors_averager_ =
    localization_common::Averager("Num Vel Prior Factors");
  // Factor Error Averagers
  localization_common::Averager of_error_averager_ = localization_common::Averager("OF Factor Error");
  localization_common::Averager loc_proj_error_averager_ = localization_common::Averager("Loc Proj Factor Error");
  localization_common::Averager loc_pose_error_averager_ = localization_common::Averager("Loc Pose Factor Error");
  localization_common::Averager imu_error_averager_ = localization_common::Averager("Imu Factor Error");
  localization_common::Averager rotation_error_averager_ = localization_common::Averager("Rotation Factor Error");
  localization_common::Averager standstill_between_error_averager_ =
    localization_common::Averager("Standstill Between Error");
  localization_common::Averager pose_prior_error_averager_ = localization_common::Averager("Pose Prior Error");
  localization_common::Averager velocity_prior_error_averager_ = localization_common::Averager("Velocity Prior Error");
  localization_common::Averager bias_prior_error_averager_ = localization_common::Averager("Bias Prior Error");
};
}  // namespace graph_localizer

#endif  // GRAPH_LOCALIZER_GRAPH_LOCALIZER_STATS_H_
