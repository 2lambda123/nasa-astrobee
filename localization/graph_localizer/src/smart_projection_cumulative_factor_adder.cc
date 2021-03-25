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

#include <graph_localizer/graph_action.h>
#include <graph_localizer/smart_projection_cumulative_factor_adder.h>
#include <graph_localizer/utilities.h>
#include <localization_common/logger.h>

#include <gtsam/base/Vector.h>
#include <gtsam/slam/SmartProjectionPoseFactor.h>

namespace graph_localizer {
namespace lm = localization_measurements;
namespace sym = gtsam::symbol_shorthand;
SmartProjectionCumulativeFactorAdder::SmartProjectionCumulativeFactorAdder(
  const SmartProjectionFactorAdderParams& params, std::shared_ptr<const FeatureTracker> feature_tracker)
    : SmartProjectionCumulativeFactorAdder::Base(params), feature_tracker_(feature_tracker) {
  smart_projection_params_.verboseCheirality = params.verbose_cheirality;
  smart_projection_params_.setRankTolerance(1e-9);
  smart_projection_params_.setLandmarkDistanceThreshold(params.landmark_distance_threshold);
  smart_projection_params_.setDynamicOutlierRejectionThreshold(params.dynamic_outlier_rejection_threshold);
  smart_projection_params_.setRetriangulationThreshold(params.retriangulation_threshold);
  smart_projection_params_.setEnableEPI(params.enable_EPI);
}

std::vector<FactorsToAdd> SmartProjectionCumulativeFactorAdder::AddFactors() {
  // Add smart factor for each valid feature track
  FactorsToAdd smart_factors_to_add(GraphAction::kDeleteExistingSmartFactors);
  int num_added_smart_factors = 0;
  const auto& feature_tracks = feature_tracker_->feature_tracks_length_ordered();
  const auto& longest_feature_track = feature_tracker_->LongestFeatureTrack();
  if (!longest_feature_track) {
    LogDebug("AddFactors: Failed to get longest feature track.");
    return {};
  }
  std::vector<lm::FeaturePoint> added_points;
  const int spacing =
    longest_feature_track->BestSpacing(params().measurement_spacing, params().max_num_points_per_factor);
  // Iterate in reverse order so longer feature tracks are prioritized
  for (auto feature_track_it = feature_tracks.crbegin(); feature_track_it != feature_tracks.crend();
       ++feature_track_it) {
    if (num_added_smart_factors >= params().max_num_factors) break;
    const auto& feature_track = *(feature_track_it->second);
    const double average_distance_from_mean = AverageDistanceFromMean(feature_track.points());
    const auto points = feature_track.LatestPoints(spacing);
    if (ValidPointSet(points.size(), average_distance_from_mean, params().min_avg_distance_from_mean,
                      params().min_num_points) &&
        !TooClose(added_points, points.front())) {
      AddSmartFactor(points, smart_factors_to_add);
      // Use latest point
      added_points.emplace_back(points.front());
      ++num_added_smart_factors;
    }
  }

  if (smart_factors_to_add.empty()) return {};
  const auto latest_timestamp = feature_tracker_->LatestTimestamp();
  if (!latest_timestamp) {
    LogError("AddFactors: Failed to get latest timestamp.");
    return {};
  }
  smart_factors_to_add.SetTimestamp(*latest_timestamp);
  LogDebug("AddFactors: Added " << smart_factors_to_add.size() << " smart factors.");
  return {smart_factors_to_add};
}

void SmartProjectionCumulativeFactorAdder::AddSmartFactor(const std::vector<lm::FeaturePoint>& feature_track_points,
                                                          FactorsToAdd& smart_factors_to_add) const {
  SharedRobustSmartFactor smart_factor;
  const int num_feature_track_points = feature_track_points.size();
  const auto noise = params().scale_noise_with_num_points
                       ? gtsam::noiseModel::Isotropic::Sigma(
                           2, params().noise_scale * num_feature_track_points * params().cam_noise->sigma())
                       : params().cam_noise;
  smart_factor =
    boost::make_shared<RobustSmartFactor>(noise, params().cam_intrinsics, params().body_T_cam, smart_projection_params_,
                                          params().rotation_only_fallback, params().robust, params().huber_k);

  KeyInfos key_infos;
  key_infos.reserve(feature_track_points.size());
  // Gtsam requires unique key indices for each key, even though these will be replaced later
  int uninitialized_key_index = 0;
  for (int i = 0; i < static_cast<int>(feature_track_points.size()); ++i) {
    const auto& feature_point = feature_track_points[i];
    if (i >= params().max_num_points_per_factor) break;
    const KeyInfo key_info(&sym::P, feature_point.timestamp);
    key_infos.emplace_back(key_info);
    smart_factor->add(Camera::Measurement(feature_point.image_point), key_info.MakeKey(uninitialized_key_index++));
  }
  smart_factors_to_add.push_back({key_infos, smart_factor});
}

bool SmartProjectionCumulativeFactorAdder::TooClose(const std::vector<lm::FeaturePoint>& added_points,
                                                    const lm::FeaturePoint& point) const {
  for (const auto& added_point : added_points) {
    if (((added_point.image_point - point.image_point).norm()) < params().feature_track_min_separation) {
      return true;
    }
  }
  return false;
}
}  // namespace graph_localizer
