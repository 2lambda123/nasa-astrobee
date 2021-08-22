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
#ifndef DEPTH_ODOMETRY_DEPTH_IMAGE_ALIGNER_PARAMS_H_
#define DEPTH_ODOMETRY_DEPTH_IMAGE_ALIGNER_PARAMS_H_

#include <camera/camera_params.h>

#include <memory>
#include <string>

namespace depth_odometry {
struct DepthImageAlignerParams {
  int max_match_hamming_distance;
  // Flann params
  int flann_table_number;
  int flann_key_size;
  int flann_multi_probe_level;
  // Feature detector params
  std::string detector;
  // Brisk params
  int brisk_threshold;
  int brisk_octaves;
  float brisk_float_pattern_scale;
  // Surf params
  int surf_threshold;
  // CLAHE params
  bool use_clahe;
  int clahe_grid_length;
  double clahe_clip_limit;
  // RansacEstimateCamera params
  int num_ransac_iterations;
  int max_inlier_tolerance;
  // Other
  int min_num_inliers;
  std::shared_ptr<camera::CameraParameters> camera_params;
};
}  // namespace depth_odometry

#endif  // DEPTH_ODOMETRY_DEPTH_IMAGE_ALIGNER_PARAMS_H_
