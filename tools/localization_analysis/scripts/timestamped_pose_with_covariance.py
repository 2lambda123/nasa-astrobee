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

from pose_with_covariance import PoseWithCovariance 

# Class that contains a timestamped pose and covariance.
class TimestampedPoseWithCovariance(PoseWithCovariance):
    def __init__(self, orientation, position, covariance, timestamp):
        super(TimestampedPoseWithCovariance, self).__init__(orientation, position)
        self.timestamp = timestamp
