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
#ifndef CALIBRATION_CAMERA_TARGET_BASED_INTRINSICS_CALIBRATOR_H_
#define CALIBRATION_CAMERA_TARGET_BASED_INTRINSICS_CALIBRATOR_H_

#include <calibration/camera_target_based_intrinsics_calibrator_params.h>
#include <calibration/utilities.h>
#include <ff_common/eigen_vectors.h>
#include <localization_common/image_correspondences.h>
#include <localization_common/logger.h>
#include <optimization_common/residuals.h>
#include <optimization_common/utilities.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <ceres/ceres.h>
#include <ceres/solver.h>

#include <utility>
#include <vector>

namespace calibration {
struct MatchSet {
  MatchSet(const localization_common::ImageCorrespondences& correspondences, const Eigen::Isometry3d& pose_estimate,
           const std::vector<int>& inliers)
      : correspondences(correspondences), pose_estimate(pose_estimate), inliers(inliers) {}
  localization_common::ImageCorrespondences correspondences;
  Eigen::Isometry3d pose_estimate;
  std::vector<int> inliers;
};

struct OptimizationStateParameters {
  Eigen::Vector2d focal_lengths;
  Eigen::Vector2d principal_points;
  Eigen::VectorXd distortion;
  std::vector<Eigen::Matrix<double, 6, 1>> camera_T_targets;
};

struct StateParameters {
  Eigen::Vector2d focal_lengths;
  Eigen::Vector2d principal_points;
  Eigen::VectorXd distortion;
  std::vector<Eigen::Isometry3d> camera_T_targets;
};

template <typename DISTORTER>
class CameraTargetBasedIntrinsicsCalibrator {
 public:
  explicit CameraTargetBasedIntrinsicsCalibrator(const CameraTargetBasedIntrinsicsCalibratorParams& params)
      : params_(params) {}

  void EstimateInitialTargetPosesAndCalibrate(
    const std::vector<localization_common::ImageCorrespondences>& correspondences_set,
    const StateParameters& initial_state_parameters, StateParameters& calibrated_state_parameters);

  void Calibrate(const std::vector<MatchSet>& match_sets, const StateParameters& initial_state_parameters,
                 StateParameters& calibrated_state_parameters);

  const CameraTargetBasedIntrinsicsCalibratorParams& params() { return params_; }

 private:
  double RadialScaleFactor(const Eigen::Vector2d& image_point, const Eigen::Vector2i& image_size) const;
  localization_common::ImageCorrespondences InlierMatches(const localization_common::ImageCorrespondences& match_set,
                                                          const std::vector<int>& inliers) const;

  CameraTargetBasedIntrinsicsCalibratorParams params_;
};

template <typename DISTORTER>
void CameraTargetBasedIntrinsicsCalibrator<DISTORTER>::EstimateInitialTargetPosesAndCalibrate(
  const std::vector<localization_common::ImageCorrespondences>& correspondences_set,
  const StateParameters& initial_state_parameters, StateParameters& calibrated_state_parameters) {
  std::vector<MatchSet> match_sets;
  for (const auto& correspondences : correspondences_set) {
    const auto camera_T_target = ReprojectionPoseEstimate<DISTORTER>(
      correspondences.image_points, correspondences.points_3d, initial_state_parameters.focal_lengths,
      initial_state_parameters.principal_points, initial_state_parameters.distortion,
      params_.reprojection_pose_estimate);

    if (!camera_T_target) {
      LogError("Failed to get camera_T_target with " << correspondences.points_3d.size() << " matches.");
      continue;
    }

    match_sets.emplace_back(correspondences, camera_T_target->first, camera_T_target->second);
  }

  Calibrate(match_sets, initial_state_parameters, calibrated_state_parameters);
}

template <typename DISTORTER>
void CameraTargetBasedIntrinsicsCalibrator<DISTORTER>::Calibrate(const std::vector<MatchSet>& match_sets,
                                                                 const StateParameters& initial_state_parameters,
                                                                 StateParameters& calibrated_state_parameters) {
  // TODO(rsoussan): add optimization state parameters struct!
  // add functions to optimization struct to initialize with state parameters struct and
  // to set final parameters to state parameters struct!
  Eigen::Vector2d focal_lengths = initial_state_parameters.focal_lengths;
  Eigen::Vector2d principal_points = initial_state_parameters.principal_points;
  Eigen::VectorXd distortion = initial_state_parameters.distortion;

  ceres::Problem problem;
  optimization_common::AddParameterBlock(2, focal_lengths.data(), problem, !params_.calibrate_focal_lengths);
  optimization_common::AddParameterBlock(2, principal_points.data(), problem, !params_.calibrate_principal_points);
  optimization_common::AddParameterBlock(DISTORTER::kNumParams, distortion.data(), problem,
                                         !params_.calibrate_distortion);
  if (params_.calibrate_focal_lengths) LogInfo("Calibrating focal lengths.");
  if (params_.calibrate_principal_points) LogInfo("Calibrating principal points.");
  if (params_.calibrate_distortion) LogInfo("Calibrating distortion.");
  if (params_.calibrate_target_poses) LogInfo("Calibrating target poses.");

  std::vector<Eigen::Matrix<double, 6, 1>> camera_T_targets;
  std::vector<Eigen::Isometry3d> initial_camera_T_targets;

  const Eigen::Matrix3d initial_intrinsics = optimization_common::Intrinsics(focal_lengths, principal_points);
  std::vector<localization_common::ImageCorrespondences> valid_correspondences_set;
  camera_T_targets.reserve(match_sets.size());
  for (const auto& match_set : match_sets) {
    camera_T_targets.emplace_back(optimization_common::VectorFromIsometry3d(match_set.pose_estimate));
    initial_camera_T_targets.emplace_back(match_set.pose_estimate);
    if (params_.save_individual_initial_reprojection_images) {
      static int image_count = 0;
      SaveReprojectionImage<DISTORTER>(match_set.correspondences.image_points, match_set.correspondences.points_3d,
                                       match_set.inliers, initial_intrinsics, distortion, match_set.pose_estimate,
                                       params_.individual_max_visualization_error_norm,
                                       "reprojection_image_" + std::to_string(image_count++) + ".png");
    }

    problem.AddParameterBlock(camera_T_targets.back().data(), 6);
    if (!params_.calibrate_target_poses) problem.SetParameterBlockConstant(camera_T_targets.back().data());
    localization_common::ImageCorrespondences valid_correspondences;
    // TODO(rsoussan): use ternary expression here
    if (params_.only_use_inliers) {
      valid_correspondences = InlierMatches(match_set.correspondences, match_set.inliers);
    } else {
      valid_correspondences = match_set.correspondences;
    }
    valid_correspondences_set.emplace_back(valid_correspondences);
    for (int i = 0; i < static_cast<int>(valid_correspondences.image_points.size()) && i < params_.max_num_match_sets;
         ++i) {
      const double radial_scale_factor = RadialScaleFactor(valid_correspondences.image_points[i], params_.image_size);
      optimization_common::ReprojectionError<DISTORTER>::AddCostFunction(
        valid_correspondences.image_points[i], valid_correspondences.points_3d[i], camera_T_targets.back(),
        focal_lengths, principal_points, distortion, problem, radial_scale_factor, params_.optimization.huber_loss);
    }
  }

  ceres::Solver::Summary summary;
  ceres::Solve(params_.optimization.solver_options, &problem, &summary);
  if (params_.optimization.verbose) std::cout << summary.FullReport() << std::endl;

  calibrated_state_parameters.focal_lengths = focal_lengths;
  calibrated_state_parameters.principal_points = principal_points;
  calibrated_state_parameters.distortion = distortion;

  // TODO(rsoussan): pass match set instead of initial camera t targets here, remove initial camera t targets
  if (params_.calibrate_target_poses) PrintCameraTTargetsStats(initial_camera_T_targets, camera_T_targets);

  // TODO(rsoussan): add option for saving final target image
  const Eigen::Matrix3d calibrated_intrinsics = optimization_common::Intrinsics(
    calibrated_state_parameters.focal_lengths, calibrated_state_parameters.principal_points);
  SaveReprojectionFromAllTargetsImage<DISTORTER>(camera_T_targets, valid_correspondences_set, calibrated_intrinsics,
                                                 distortion, params_.image_size, params_.max_visualization_error_norm);
}

template <typename DISTORTER>
double CameraTargetBasedIntrinsicsCalibrator<DISTORTER>::RadialScaleFactor(const Eigen::Vector2d& image_point,
                                                                           const Eigen::Vector2i& image_size) const {
  if (!params_.scale_loss_radially) return 1.0;
  const Eigen::Vector2d centered_image_point = image_point - image_size.cast<double>() / 2.0;
  const double radius = centered_image_point.norm();
  return std::pow(radius, params_.radial_scale_power);
}

template <typename DISTORTER>
localization_common::ImageCorrespondences CameraTargetBasedIntrinsicsCalibrator<DISTORTER>::InlierMatches(
  const localization_common::ImageCorrespondences& match_set, const std::vector<int>& inliers) const {
  std::vector<Eigen::Vector2d> inlier_image_points;
  std::vector<Eigen::Vector3d> inlier_points_3d;
  for (const int inlier_index : inliers) {
    inlier_image_points.emplace_back(match_set.image_points[inlier_index]);
    inlier_points_3d.emplace_back(match_set.points_3d[inlier_index]);
  }
  return localization_common::ImageCorrespondences(inlier_image_points, inlier_points_3d);
}
}  // namespace calibration

#endif  // CALIBRATION_CAMERA_TARGET_BASED_INTRINSICS_CALIBRATOR_H_
