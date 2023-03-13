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

#ifndef GRAPH_OPTIMIZER_UTILITIES_H_
#define GRAPH_OPTIMIZER_UTILITIES_H_

#include <localization_common/logger.h>

#include <gtsam/linear/NoiseModel.h>
#include <gtsam/nonlinear/NonlinearFactorGraph.h>

#include <vector>

namespace graph_optimizer {
gtsam::noiseModel::Robust::shared_ptr Robust(const gtsam::SharedNoiseModel& noise, const double huber_k);

gtsam::NonlinearFactorGraph RemoveOldFactors(const gtsam::KeyVector& old_keys, gtsam::NonlinearFactorGraph& graph);

template <typename FactorType>
void DeleteFactors(gtsam::NonlinearFactorGraph& graph) {
  int num_removed_factors = 0;
  for (auto factor_it = graph.begin(); factor_it != graph.end();) {
    if (dynamic_cast<FactorType*>(factor_it->get())) {
      factor_it = graph.erase(factor_it);
      ++num_removed_factors;
      continue;
    }
    ++factor_it;
  }
  LogDebug("DeleteFactors: Num removed factors: " << num_removed_factors);
}

template <typename FactorType>
int NumFactors(const gtsam::NonlinearFactorGraph& graph) {
  int num_factors = 0;
  for (const auto& factor : graph) {
    if (dynamic_cast<const FactorType*>(factor.get())) {
      ++num_factors;
    }
  }
  return num_factors;
}

template <typename FactorType>
std::vector<const FactorType*> Factors(const gtsam::NonlinearFactorGraph& graph) {
  std::vector<const FactorType*> factors;
  for (const auto& factor : graph) {
    const FactorType* casted_factor = dynamic_cast<const FactorType*>(factor.get());
    if (casted_factor) {
      factors.emplace_back(casted_factor);
    }
  }
  return factors;
}
}  // namespace graph_optimizer

#endif  // GRAPH_OPTIMIZER_UTILITIES_H_
