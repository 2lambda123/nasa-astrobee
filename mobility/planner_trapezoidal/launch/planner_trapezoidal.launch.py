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

from utilities.utilities import *


def generate_launch_description():

    return LaunchDescription([
        DeclareLaunchArgument("name", default_value="planner_trapezoidal"),
        DeclareLaunchArgument("manager", default_value=""),

        IncludeLaunchDescription(
            get_launch_file('$launch/ff_component.launch.py'),
            launch_arguments = {
                                "name"   : LaunchConfiguration("name"),
                                "manager": LaunchConfiguration("manager"),
                                "name"   : "planner_trapezoidal/PlannerTrapezoidalComponent",
                                "debug"  : "false"
            }
        )
    ])