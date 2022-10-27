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

#ifndef GNC_AUTOCODE_CTL_H_
#define GNC_AUTOCODE_CTL_H_

#include <Eigen/Dense>

extern "C" {
#include <ctl_controller0.h>
}

namespace config_reader {
  class ConfigReader;
}

namespace constants {
  // TODO(bcoltin): load
const unsigned int ase_status_converged = 0U;
const unsigned int ctl_idle_mode = 0U;
const unsigned int ctl_stopping_mode = 1U;
const unsigned int ctl_stopped_mode = 3U;
}  // namespace constants

namespace gnc_autocode {

class Control {
 public:
  Control(void);

  virtual void Initialize(void);
  virtual void Step(void);
  virtual void ReadParams(config_reader::ConfigReader* config);

  ctl_input_msg ctl_input_;

  // outputs
  cmd_msg cmd_;
  ctl_msg ctl_;

 private:
  int mode_cmd;
  bool stopped_mode;
  float prev_filter_vel[3];
  float prev_filter_omega[3];
  int prev_mode_cmd[5];  // for the 4 ticks required  to switch to stopped; newest val at index 0
  float prev_position[3];
  float prev_att[4];
  float pos_err_parameter[3];  // in cex control executive
  float quat_err;

  // Simulink outports
  // cex_control_executive
  float att_command[4];
  float ctl_status;
  float omega_command[3];
  float alpha_command[3];

  // clc_closed_loop controller
  Eigen::Vector3f Kp_lin;
  Eigen::Vector3f Ki_lin;
  Eigen::Vector3f Kd_lin;
  float Kp_rot[3];
  float Ki_rot[3];
  float Kd_rot[3];
  float body_accel_cmd[3];
  float body_force_cmd[3];
  Eigen::Vector3f pos_err_outport;  // in closed loop controller
  Eigen::Vector3f CMD_P_B_ISS_ISS;
  Eigen::Vector3f CMD_V_B_ISS_ISS;
  Eigen::Vector3f CMD_A_B_ISS_ISS;
  float CMD_Quat_ISS2B[4];
  float CMD_Omega_B_ISS_B[3];
  float CMD_Alpha_B_ISS_B[3];
  float linear_integrator[3];
  float rotational_integrator[3];  // accumulator
  Eigen::Vector3f linear_int_err;
  float att_err_mag;
  float att_err[3];
  float dummy[3];
  float rotate_int_err[3];
  float body_alpha_cmd[3];
  Eigen::Matrix<float, 3, 3> inertia;
  Eigen::Vector3f rate_error;  // helper in rot control
  float body_torque_cmd[3];
  float traj_pos_previous[3];

  // command shaper
  bool cmd_B_inuse;
  bool state_cmd_switch_out;
  float cmd_timestamp_sec;
  float cmd_timestamp_nsec;
  float time_delta;
  float traj_pos[3];
  float traj_vel[3];
  float traj_accel[3];
  float traj_quat[4];
  float traj_omega[3];
  float traj_alpha[3];
  float traj_error_pos;
  float traj_error_att;
  float traj_error_vel;
  float traj_error_omega;

  bool BelowThreshold(float velocity[], float threshhold, float previous[3]);
  void UpdateModeCmd(void);
  float ButterWorthFilter(float input, float& delay_val);
  bool CmdModeMakeCondition();
  void UpdateStoppedMode();
  void UpdatePosAndQuat();
  void FindPosError();
  void UpdatePrevious();
  void FindQuatError(float q_cmd[4], float q_actual[4], float& output, float output_vec[3]);
  void UpdateCtlStatus();
  bool CtlStatusSwitch();
  void BypassShaper();
  // clc_closed_loop controller
  void VariablesTransfer();
  float SafeDivide(float num, float denom);
  void UpdateLinearPIDVals();
  void FindPosErr();
  Eigen::Vector3f discreteTimeIntegrator(Eigen::Vector3f input, float accumulator[3], float upper_limit,
                              float lower_limit);
  void FindLinearIntErr();
  void FindBodyForceCmd();
  void SkewSymetricMatrix(const float input[3], float output[3][3]);
  void QuaternionToDCM(float input_quat[4], float output[3][3]);
  Eigen::Vector3f RotateVectorAtoB(Eigen::Vector3f, Eigen::Quaternionf);
  void MatrixMultiplication3x3(float inputA[3][3], float inputB[3][3], float output[3][3]);
  Eigen::Vector3f SaturateVector(Eigen::Vector3f, float limit);
  void UpdateRotationalPIDVals();
  void UpdateRotateIntErr();
  void MatrixMultiplication3x1(float three[3][3], float one[3], float output[3]);
  void FindBodyAlphaCmd();
  Eigen::Vector3f AngAccelHelper();
  void FindBodyTorqueCmd();
  void CrossProduct(float vecA[3], float vecB[3], float vecOut[3]);
  void FindAttErr();

  void VarToCtlMsg();

// command shaper
  void CmdSelector();
  void GenerateCmdPath();
  void GenerateCmdAttitude();
  void FindTrajQuat(float omega_B_ISS_B[3], float alpha_B_ISS_B[3], float quat_ISS2B[4]);
  void MatrixMultiplication4x4(float inputA[4][4], float inputB[4][4], float output[4][4]);
  void MatrixMultiplication4x1(float four[4][4], float one[4], float output[4]);
  void FindTrajErrors(float traj_q[4]);
  void PublishCmdInput();

  Eigen::Matrix<float, 4, 4> OmegaMatrix(float input[3]);

  Eigen::Vector3f tun_accel_gain;
  Eigen::Vector3f tun_alpha_gain;
  float tun_ctl_stopping_omega_thresh;
  float tun_ctl_stopping_vel_thresh;
  float tun_ctl_stopped_pos_thresh;
  float tun_ctl_stopped_quat_thresh;
  float tun_ctl_pos_sat_upper;
  float tun_ctl_pos_sat_lower;
  float tun_ctl_linear_force_limit;
  float tun_ctl_att_sat_upper;
  float tun_ctl_att_sat_lower;
};
}  // end namespace gnc_autocode

#endif  // GNC_AUTOCODE_CTL_H_

