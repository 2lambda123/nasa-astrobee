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

#ifndef VISION_COMMON_FEATURE_TRACKER_H_
#define VISION_COMMON_FEATURE_TRACKER_H_

#include <localization_common/time.h>
#include <vision_common/feature_point.h>
#include <vision_common/feature_track.h>
#include <vision_common/feature_tracker_params.h>

#include <map>

namespace vision_common {
using FeatureTrackIdMap = std::map<FeatureId, std::shared_ptr<FeatureTrack>>;
class FeatureTracker {
 public:
  explicit FeatureTracker(const FeatureTrackerParams& params);

  // Default constructor only for serialization
  FeatureTracker() = default;

  ~FeatureTracker() = default;

  // Add new feature points to existing or new tracks.  Optionally removes
  // any existing tracks that weren't detected in passed feature_points.
  void UpdateFeatureTracks(const FeaturePoints& feature_points);

  // Remove any points older than oldest_allowed_time from each feature track.
  void RemoveOldPoints(const localization_common::Time oldest_allowed_time);

  // Returns a reference to the feature tracks.
  const FeatureTrackIdMap& feature_tracks() const;

  // Returns the number of feature tracks.
  size_t size() const;

  // Returns if no feature tracks exist.
  bool empty() const;

  // Deletes all feature tracks.
  void Clear();

 private:
  // Add new feature point to an existing track with the same track id
  // create a new track if this doesn't exist.
  void AddOrUpdateTrack(const FeaturePoint& feature_point);

  // Remove any feature tracks without detections at the provided timestamp
  void RemoveUndetectedFeatureTracks(const localization_common::Time& time);

  // Serialization function
  friend class boost::serialization::access;
  template <class ARCHIVE>
  void serialize(ARCHIVE& ar, const unsigned int /*version*/) {
    ar& BOOST_SERIALIZATION_NVP(feature_track_id_map_);
    ar& BOOST_SERIALIZATION_NVP(params_);
  }

  FeatureTrackIdMap feature_track_id_map_;
  FeatureTrackerParams params_;
};
}  // namespace vision_common

#endif  // VISION_COMMON_FEATURE_TRACKER_H_
