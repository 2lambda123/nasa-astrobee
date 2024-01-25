#!/usr/bin/python
#
# Copyright (c) 2017, United States Government, as represented by the
# Administrator of the National Aeronautics and Space Administration.
#
# All rights reserved.
#
# The Astrobee platform is licensed under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with the
# License. You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

# GraphVIOState object containing information from a GraphVIOState Msg
class GraphVIOState():
    def __init__(self):
        self.timestamp = None
        self.pose_with_covariance = None
        #self.velocity_with_covariance = message_conversions.velocity_from_msg(msg.velocity, bag_start_time)
        #self.imu_bias_with_covariance = ImuBias(msg.accel_bias, msg.gyro_bias)
        #self.num_detected_of_features = msg.num_detected_of_features
        #self.num_of_factors = msg.num_of_factors
        # TODO: change this using bag start time??
