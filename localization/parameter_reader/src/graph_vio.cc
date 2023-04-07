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

#include <parameter_reader/factor_adders.h>
#include <parameter_reader/graph_vio.h>
#include <parameter_reader/node_adders.h>
#include <parameter_reader/optimizers.h>
#include <parameter_reader/sliding_window_graph_optimizer.h>
#include <parameter_reader/vision_common.h>
#include <msg_conversions/msg_conversions.h>

namespace parameter_reader {
namespace gv = graph_vio;
namespace mc = msg_conversions;

void LoadGraphVIOParams(config_reader::ConfigReader& config, gv::GraphVIOParams& params,
                              const std::string& prefix) {
  LoadStandstillFactorAdderParams(params.standstill_factor_adder, config, prefix);
  LoadVoSmartProjectionFactorAdderParams(params.vo_smart_projection_factor_adder, config, prefix);
  LoadCombinedNavStateNodeAdder::Params(params.combined_nav_state_node_adder, config, prefix);
  LoadCombinedNavStateNodeAdderModelParams(params.combined_nav_state_node_adder_model, config, prefix);
  LoadNonlinearOptimizerParams(params.nonlinear_optimizer, config, prefix);
  LoadSlidingWindowGraphOptimizerParams(params.sliding_window_graph_optimizer, config, prefix);
  LoadStandstillParams(params.standstill, config, prefix);
}
}  // namespace parameter_reader
