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

#ifndef LOCALIZATION_COMMON_TIMESTAMPED_SET_H_
#define LOCALIZATION_COMMON_TIMESTAMPED_SET_H_

#include <localization_common/logger.h>
#include <localization_common/time.h>

#include <boost/optional.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/unordered_map.hpp>

#include <algorithm>
#include <map>
#include <utility>
#include <vector>

namespace localization_common {
template <typename T>
struct TimestampedValue {
  TimestampedValue(const Time timestamp, const T& value) : timestamp(timestamp), value(value) {}
  explicit TimestampedValue(const std::pair<const Time, T>& pair) : timestamp(pair.first), value(pair.second) {}
  Time timestamp;
  // TODO(rsoussan): Store this as const ref? Make it optional to be ref or not?
  T value;
};

template <typename T>
class TimestampedSet {
 public:
  TimestampedSet();
  ~TimestampedSet() = default;

  // Assumes values have corresponding timestamps at the same index and each timestamp is unique.
  TimestampedSet(const std::vector<Time>& timestamps, const std::vector<T>& values);

  bool Add(const Time timestamp, const T& value);

  bool Remove(const Time timestamp);

  boost::optional<TimestampedValue<T>> Get(const Time timestamp) const;

  size_t size() const;

  bool empty() const;

  boost::optional<TimestampedValue<T>> Oldest() const;

  boost::optional<TimestampedValue<T>> Latest() const;

  bool WithinBounds(const Time timestamp) const;

  // Return lower and upper bounds.  Equal values are set as both lower and upper bound.
  std::pair<boost::optional<TimestampedValue<T>>, boost::optional<TimestampedValue<T>>> LowerAndUpperBound(
    const Time timestamp) const;

  boost::optional<TimestampedValue<T>> Closest(const Time timestamp) const;

  boost::optional<TimestampedValue<T>> LowerBoundOrEqual(const Time timestamp) const;

  std::vector<Time> Timestamps() const;

  double Duration() const;

  bool Contains(const Time timestamp) const;

  // Returns the latest values not older than the provided oldest allowed timestamp.
  // Orders values from older to later values.
  std::vector<TimestampedValue<T>> LatestValues(const Time oldest_allowed_timestamp) const;

  std::vector<TimestampedValue<T>> OldValues(const Time oldest_allowed_timestamp) const;

  // Returns values occuring at the provided timestamps.
  // Orders values from older to later values.
  template <typename TimestampSetType>
  std::vector<TimestampedValue<T>> DownsampledValues(const TimestampSetType& allowed_timestamps) const;

  int RemoveOldValues(const Time oldest_allowed_timestamp);

  // Finds the lower bound element for timestamp and removes all values below this.
  // Keeps the lower bound, unlike RemoveOldValues which only keeps a lower bound
  // equal to the provided timestamp.
  int RemoveBelowLowerBoundValues(const Time timestamp);

  const std::map<Time, T>& set() const;

  typename std::map<Time, T>::const_iterator cend() const;

  // Returns iterators to values in range of oldest and latest allowed timestamps.
  // The second iterator is one-past the latest allowed element to allow for iterating
  // using: for(auto it = pair.first; it != pair.second; ++it)
  std::pair<typename std::map<Time, T>::const_iterator, typename std::map<Time, T>::const_iterator> InRangeValues(
    const Time oldest_allowed_timestamp, const Time latest_allowed_timestamp);

 private:
  friend class boost::serialization::access;
  template <class ARCHIVE>
  void serialize(ARCHIVE& ar, const unsigned int /*version*/);

  std::map<Time, T> timestamp_value_map_;
};

// Implementation
template <typename T>
TimestampedSet<T>::TimestampedSet() {}

template <typename T>
TimestampedSet<T>::TimestampedSet(const std::vector<Time>& timestamps, const std::vector<T>& values) {
  for (int i = 0; i < values.size(); ++i) {
    Add(timestamps[i], values[i]);
  }
}

template <typename T>
bool TimestampedSet<T>::Add(const Time timestamp, const T& value) {
  if (Contains(timestamp)) return false;
  timestamp_value_map_.emplace(timestamp, value);
  return true;
}

template <typename T>
bool TimestampedSet<T>::Remove(const Time timestamp) {
  if (!Contains(timestamp)) return false;
  timestamp_value_map_.erase(timestamp);
  return true;
}

template <typename T>
boost::optional<TimestampedValue<T>> TimestampedSet<T>::Get(const Time timestamp) const {
  if (!Contains(timestamp)) return boost::none;
  return TimestampedValue<T>(timestamp, timestamp_value_map_.at(timestamp));
}

template <typename T>
size_t TimestampedSet<T>::size() const {
  return timestamp_value_map_.size();
}

template <typename T>
bool TimestampedSet<T>::empty() const {
  return timestamp_value_map_.empty();
}

template <typename T>
boost::optional<TimestampedValue<T>> TimestampedSet<T>::Oldest() const {
  if (empty()) {
    LogDebug("Oldest: No timestamps available.");
    return boost::none;
  }
  return TimestampedValue<T>(*timestamp_value_map_.cbegin());
}

template <typename T>
boost::optional<TimestampedValue<T>> TimestampedSet<T>::Latest() const {
  if (empty()) {
    LogDebug("Latest: No values available.");
    return boost::none;
  }
  return TimestampedValue<T>(*timestamp_value_map_.crbegin());
}

template <typename T>
bool TimestampedSet<T>::WithinBounds(const Time time) const {
  const auto oldest = Oldest();
  const auto latest = Latest();
  if (!oldest || !latest) {
    LogError("WithinBounds: Failed to get time bounds.");
    return false;
  }
  return (time >= oldest->timestamp && time <= latest->timestamp);
}

template <typename T>
std::pair<boost::optional<TimestampedValue<T>>, boost::optional<TimestampedValue<T>>>
TimestampedSet<T>::LowerAndUpperBound(const Time timestamp) const {
  if (empty()) {
    LogDebug("LowerAndUpperBound: No timestamps available.");
    return {boost::none, boost::none};
  }

  // lower bound returns first it >= query, call this upper bound
  const auto upper_bound_it = timestamp_value_map_.lower_bound(timestamp);
  if (upper_bound_it == timestamp_value_map_.cend()) {
    LogDebug("LowerAndUpperBoundTimestamps: No upper bound timestamp exists.");
    return {boost::optional<TimestampedValue<T>>(*(timestamp_value_map_.crbegin())), boost::none};
  } else if (upper_bound_it->first == timestamp) {
    // Handle equality, set lower and upper bound to same elements
    return {boost::optional<TimestampedValue<T>>(*upper_bound_it),
            boost::optional<TimestampedValue<T>>(*upper_bound_it)};
  } else if (upper_bound_it == timestamp_value_map_.cbegin()) {
    LogDebug("LowerAndUpperBoundTimestamps: No lower bound timestamp exists.");
    return {boost::none, boost::optional<TimestampedValue<T>>(*upper_bound_it)};
  }
  const auto lower_bound_it = std::prev(upper_bound_it);

  return {boost::optional<TimestampedValue<T>>(*lower_bound_it), boost::optional<TimestampedValue<T>>(*upper_bound_it)};
}

template <typename T>
boost::optional<TimestampedValue<T>> TimestampedSet<T>::LowerBoundOrEqual(const Time timestamp) const {
  const auto lower_and_upper_bound_values = LowerAndUpperBound(timestamp);
  if (!lower_and_upper_bound_values.first && !lower_and_upper_bound_values.second) {
    LogDebug("LowerBoundOrEqual: Failed to get lower or upper bound values.");
    return boost::none;
  }

  // Only return upper bound values if it is equal to timestamp
  if (lower_and_upper_bound_values.second && lower_and_upper_bound_values.second->timestamp == timestamp) {
    return lower_and_upper_bound_values.second;
  }

  return lower_and_upper_bound_values.first;
}

template <typename T>
std::vector<Time> TimestampedSet<T>::Timestamps() const {
  std::vector<Time> timestamps;
  for (const auto& timestamped_value : timestamp_value_map_) {
    timestamps.emplace_back(timestamped_value.first);
  }
  return timestamps;
}

template <typename T>
double TimestampedSet<T>::Duration() const {
  if (empty()) return 0;
  return (Latest()->timestamp - Oldest()->timestamp);
}

template <typename T>
boost::optional<TimestampedValue<T>> TimestampedSet<T>::Closest(const Time timestamp) const {
  if (empty()) {
    LogDebug("Closest: No values available.");
    return boost::none;
  }

  const auto lower_and_upper_bound_values = LowerAndUpperBound(timestamp);
  if (!lower_and_upper_bound_values.first && !lower_and_upper_bound_values.second) {
    LogDebug("Closest: Failed to get lower or upper bound values.");
    return boost::none;
  }

  if (!lower_and_upper_bound_values.first) {
    return lower_and_upper_bound_values.second;
  } else if (!lower_and_upper_bound_values.second) {
    return lower_and_upper_bound_values.first;
  } else {
    const Time lower_bound_timestamp = lower_and_upper_bound_values.first->timestamp;
    const Time upper_bound_timestamp = lower_and_upper_bound_values.second->timestamp;
    const double upper_bound_dt = std::abs(timestamp - upper_bound_timestamp);
    const double lower_bound_dt = std::abs(timestamp - lower_bound_timestamp);
    return (upper_bound_dt < lower_bound_dt) ? lower_and_upper_bound_values.second : lower_and_upper_bound_values.first;
  }
}

template <typename T>
bool TimestampedSet<T>::Contains(const Time timestamp) const {
  return timestamp_value_map_.count(timestamp) > 0;
}

template <typename T>
std::vector<TimestampedValue<T>> TimestampedSet<T>::LatestValues(const Time oldest_allowed_timestamp) const {
  std::vector<TimestampedValue<T>> latest_values;
  std::transform(timestamp_value_map_.lower_bound(oldest_allowed_timestamp), timestamp_value_map_.end(),
                 std::back_inserter(latest_values),
                 [](const std::pair<Time, T>& value) { return TimestampedValue<T>(value); });
  return latest_values;
}

template <typename T>
std::vector<TimestampedValue<T>> TimestampedSet<T>::OldValues(const Time oldest_allowed_timestamp) const {
  std::vector<TimestampedValue<T>> old_values;
  std::transform(timestamp_value_map_.begin(), timestamp_value_map_.lower_bound(oldest_allowed_timestamp),
                 std::back_inserter(old_values),
                 [](const std::pair<Time, T>& value) { return TimestampedValue<T>(value); });
  return old_values;
}

template <typename T>
template <typename TimestampSetType>
std::vector<TimestampedValue<T>> TimestampedSet<T>::DownsampledValues(
  const TimestampSetType& allowed_timestamps) const {
  std::vector<TimestampedValue<T>> downsampled_values;
  for (const auto& pair : timestamp_value_map_) {
    if (allowed_timestamps.count(pair.first) > 0) {
      downsampled_values.emplace_back(TimestampedValue<T>(pair));
    }
  }
  return downsampled_values;
}

template <typename T>
int TimestampedSet<T>::RemoveOldValues(const Time oldest_allowed_timestamp) {
  const int initial_num_values = size();
  timestamp_value_map_.erase(timestamp_value_map_.begin(), timestamp_value_map_.lower_bound(oldest_allowed_timestamp));
  return initial_num_values - size();
}

template <typename T>
int TimestampedSet<T>::RemoveBelowLowerBoundValues(const Time timestamp) {
  const int initial_num_values = size();
  const auto upper_bound = timestamp_value_map_.lower_bound(timestamp);
  const auto lower_bound =
    upper_bound != timestamp_value_map_.cbegin() ? std::prev(upper_bound) : timestamp_value_map_.cbegin();
  timestamp_value_map_.erase(timestamp_value_map_.begin(), lower_bound);
  return initial_num_values - size();
}

template <typename T>
const std::map<Time, T>& TimestampedSet<T>::set() const {
  return timestamp_value_map_;
}

template <typename T>
typename std::map<Time, T>::const_iterator TimestampedSet<T>::cend() const {
  return timestamp_value_map_.cend();
}

template <typename T>
std::pair<typename std::map<Time, T>::const_iterator, typename std::map<Time, T>::const_iterator>
TimestampedSet<T>::InRangeValues(const Time oldest_allowed_timestamp, const Time latest_allowed_timestamp) {
  auto upper_bound = timestamp_value_map_.upper_bound(latest_allowed_timestamp);
  auto lower_bound = timestamp_value_map_.lower_bound(oldest_allowed_timestamp);
  // No values less than latest allowed time
  if (upper_bound == timestamp_value_map_.cbegin()) return {cend(), cend()};
  return std::make_pair(timestamp_value_map_.lower_bound(oldest_allowed_timestamp), upper_bound);
}

template <typename T>
template <class ARCHIVE>
void TimestampedSet<T>::serialize(ARCHIVE& ar, const unsigned int /*version*/) {
  ar& BOOST_SERIALIZATION_NVP(timestamp_value_map_);
}
}  // namespace localization_common

#endif  // LOCALIZATION_COMMON_TIMESTAMPED_SET_H_
