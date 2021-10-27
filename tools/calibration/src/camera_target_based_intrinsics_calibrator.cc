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

#include <calibration/camera_target_based_intrinsics_calibrator.h>
#include <calibration/utilities.h>
#include <localization_common/logger.h>
#include <optimization_common/fov_distortion.h>
#include <optimization_common/radtan_distortion.h>
#include <optimization_common/residuals.h>
#include <optimization_common/utilities.h>

#include <ceres/ceres.h>
#include <ceres/solver.h>

namespace calibration {
namespace lc = localization_common;
namespace oc = optimization_common;

void CameraTargetBasedIntrinsicsCalibrator::Calibrate(
  const std::vector<lc::ImageCorrespondences>& match_sets, const camera::CameraParameters& camera_params,
  const Eigen::Vector2d& initial_focal_lengths, const Eigen::Vector2d& initial_principal_points,
  const Eigen::VectorXd& initial_distortion, Eigen::Vector2d& calibrated_focal_lengths,
  Eigen::Vector2d& calibrated_principal_points, Eigen::VectorXd& calibrated_distortion) {
  Eigen::Vector2d focal_lengths = initial_focal_lengths;
  Eigen::Vector2d principal_points = initial_principal_points;
  Eigen::VectorXd distortion = initial_distortion;

  ceres::Problem problem;
  problem.AddParameterBlock(focal_lengths.data(), 2);
  problem.AddParameterBlock(principal_points.data(), 2);
  problem.AddParameterBlock(distortion.data(), distortion.size());
  if (params_.calibrate_focal_lengths)
    LogError("Calibrating focal lengths.");
  else
    problem.SetParameterBlockConstant(focal_lengths.data());
  if (params_.calibrate_principal_points)
    LogError("Calibrating principal points.");
  else
    problem.SetParameterBlockConstant(principal_points.data());
  if (params_.calibrate_distortion)
    LogError("Calibrating distortion.");
  else
    problem.SetParameterBlockConstant(distortion.data());
  if (params_.calibrate_target_poses) LogError("Calibrating target poses.");

  std::vector<Eigen::Matrix<double, 6, 1>> camera_T_targets;
  // TODO(rsoussan): More efficient way to do this
  std::vector<lc::ImageCorrespondences> valid_match_sets;
  camera_T_targets.reserve(match_sets.size());
  for (const auto& match_set : match_sets) {
    const auto& camera_T_target = CameraTTarget(camera_params, match_set, params_.min_num_target_inliers);
    if (!camera_T_target) {
      LogError("Failed to get camera_T_target with " << match_set.points_3d.size() << " matches.");
      continue;
    }
    camera_T_targets.emplace_back(oc::VectorFromIsometry3d(*camera_T_target));
    valid_match_sets.emplace_back(match_set);
    problem.AddParameterBlock(camera_T_targets.back().data(), 6);
    if (!params_.calibrate_target_poses) problem.SetParameterBlockConstant(camera_T_targets.back().data());
    for (int i = 0; i < static_cast<int>(match_set.image_points.size()) && i < params_.max_num_match_sets; ++i) {
      const double radial_scale_factor = RadialScaleFactor(match_set.image_points[i], params_.image_size);
      if (params_.distortion_type == "fov") {
        oc::AddReprojectionCostFunction<oc::FovDistortion>(
          match_set.image_points[i], match_set.points_3d[i], camera_T_targets.back(), focal_lengths, principal_points,
          distortion, problem, radial_scale_factor, params_.huber_loss);
      } else if (params_.distortion_type == "radtan") {
        oc::AddReprojectionCostFunction<oc::RadTanDistortion>(
          match_set.image_points[i], match_set.points_3d[i], camera_T_targets.back(), focal_lengths, principal_points,
          distortion, problem, radial_scale_factor, params_.huber_loss);
      } else {
        LogFatal("Invalid distortion type provided.");
      }
    }
  }

  ceres::Solver::Options options;
  if (params_.linear_solver == "dense_qr") {
    options.linear_solver_type = ceres::DENSE_QR;
  } else if (params_.linear_solver == "dense_schur") {
    options.linear_solver_type = ceres::DENSE_SCHUR;
  } else if (params_.linear_solver == "sparse_normal_cholesky") {
    options.linear_solver_type = ceres::SPARSE_NORMAL_CHOLESKY;
  } else if (params_.linear_solver == "sparse_schur") {
    options.linear_solver_type = ceres::SPARSE_SCHUR;
  } else if (params_.linear_solver == "iterative_schur") {
    options.linear_solver_type = ceres::ITERATIVE_SCHUR;
  } else {
    LogFatal("Invalid linear solver provided.");
  }
  options.use_explicit_schur_complement = params_.use_explicit_schur_complement;
  options.max_num_iterations = params_.max_num_iterations;
  options.function_tolerance = params_.function_tolerance;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  std::cout << summary.FullReport() << "\n";

  calibrated_focal_lengths = focal_lengths;
  calibrated_principal_points = principal_points;
  calibrated_distortion = distortion;
  const Eigen::Matrix3d calibrated_intrinsics = oc::Intrinsics(calibrated_focal_lengths, calibrated_principal_points);
  if (params_.distortion_type == "fov") {
    SaveReprojectionErrors<oc::FovDistortion>(camera_T_targets, valid_match_sets, calibrated_intrinsics, distortion,
                                              params_.image_size);
  } else if (params_.distortion_type == "radtan") {
    SaveReprojectionErrors<oc::RadTanDistortion>(camera_T_targets, valid_match_sets, calibrated_intrinsics, distortion,
                                                 params_.image_size);
  } else {
    LogFatal("Invalid distortion type provided.");
  }
}

double CameraTargetBasedIntrinsicsCalibrator::RadialScaleFactor(const Eigen::Vector2d& image_point,
                                                                const Eigen::Vector2i& image_size) const {
  if (!params_.scale_loss_radially) return 1.0;
  const Eigen::Vector2d centered_image_point = image_point - image_size.cast<double>() / 2.0;
  const double radius = centered_image_point.norm();
  return params_.radial_scale_factor * radius;
}
}  // namespace calibration
