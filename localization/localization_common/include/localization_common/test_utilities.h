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

#ifndef LOCALIZATION_COMMON_TEST_UTILITIES_H_
#define LOCALIZATION_COMMON_TEST_UTILITIES_H_

#include <gtsam/geometry/Pose3.h>

namespace localization_common {
// Selected from [-100, 100]
double RandomDouble();

// Selected from [0, 100]
double RandomPositiveDouble();

// Selected from [min, max]
double RandomDouble(const double min, const double max);

// Each index ranges from [-100, 100]
Eigen::Vector3d RandomVector();

// Translation ranges from [-100, 100]
// Rotation spans all possible rotations
gtsam::Pose3 RandomPose();

Eigen::Isometry3d RandomIsometry3d();

Eigen::Affine3d RandomAffine3d();

// Focal lengths and principal points selected from [0.1, 1000]
Eigen::Matrix3d RandomIntrinsics();
}  // namespace localization_common
#endif  // LOCALIZATION_COMMON_TEST_UTILITIES_H_
