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

#include "test_utilities.h"
#include <graph_localizer/silu.h>
#include <localization_common/logger.h>

#include <gtsam/base/numericalDerivative.h>

#include <gtest/gtest.h>

namespace gl = graph_localizer;
TEST(SiluTester, Jacobian) {
  for (int i = 0; i < 500; ++i) {
    const double x = gl::RandomDouble();
    gtsam::Matrix H;
    gl::Silu(x, H);
    const auto numerical_H = gtsam::numericalDerivative11<double, double>(
      boost::function<double(const double)>(boost::bind(&gl::Silu, _1, boost::none)), x);
    ASSERT_TRUE(numerical_H.isApprox(H.matrix(), 1e-6));
  }
}

TEST(SiluWithOffsetTester, Jacobian) {
  for (int i = 0; i < 500; ++i) {
    const double x = gl::RandomDouble();
    const double offset = gl::RandomDouble();
    gtsam::Matrix H;
    gl::SiluWithOffset(x, offset, H);
    const auto numerical_H = gtsam::numericalDerivative21<double, double, double>(
      boost::function<double(const double, const double)>(boost::bind(&gl::SiluWithOffset, _1, _2, boost::none)), x,
      offset);
    ASSERT_TRUE(numerical_H.isApprox(H.matrix(), 1e-6));
  }
}

TEST(SiluWithOffsetSymmetricTester, Jacobian) {
  for (int i = 0; i < 500; ++i) {
    const double x = gl::RandomDouble();
    const double offset = gl::RandomPositiveDouble();
    gtsam::Matrix H;
    const double silu_inverted_x = gl::SiluWithOffsetSymmetric(x, offset, H);
    const auto numerical_H = gtsam::numericalDerivative21<double, double, double>(
      boost::function<double(const double, const double)>(
        boost::bind(&gl::SiluWithOffsetSymmetric, _1, _2, boost::none)),
      x, offset);
    ASSERT_TRUE(gtsam::equal_with_abs_tol(numerical_H, H.matrix(), 1e-6));
  }
}
