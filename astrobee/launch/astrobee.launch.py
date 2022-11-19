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


import os

import launch
from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():

    return LaunchDescription([])


# <launch>
#   <!-- Context options (NB: THESE ARE OVERRIDDEN BY ENVIRONMENT VARIABLES)   -->
#   <!-- Set world and world correctly; environment variable over rule default -->
#   <arg name="robot" default="$(optenv ASTROBEE_ROBOT p4d)" />
#   <arg name="world" default="$(optenv ASTROBEE_WORLD granite)" />
#   <arg name="ns" default="" />                <!-- Robot namespace           -->
#   <arg name="spurn" default=""/>              <!-- PRevent a specific node   -->
#   <arg name="nodes" default=""/>              <!-- Launch specific nodes     -->
#   <arg name="extra" default=""/>              <!-- Inject an additional node -->
#   <arg name="debug" default=""/>              <!-- Debug node group          -->
#   <arg name="dds" default="true" />           <!-- Enable DDS                -->

#   <!-- Remaining options -->
#   <arg name="output" default="log" />         <!-- Output to screen or log   -->
#   <arg name="gtloc" default="false" />        <!-- Use Ground Truth Localizer -->
#   <arg name="drivers" default="true" />       <!-- Should we launch drivers? -->
#   <arg name="sim" default="local" />          <!-- SIM IP address            -->
#   <arg name="llp" default="local" />          <!-- LLP IP address            -->
#   <arg name="mlp" default="local" />          <!-- MLP IP address            -->

#   <!-- Payload options -->
#   <arg name="top_aft" default="perching_arm" /> <!-- Payload bays            -->
#   <arg name="bot_aft" default="empty" />        <!-- Payload bays            -->
#   <arg name="bot_front" default="empty" />      <!-- Payload bays            -->

#   <!-- Simulation options only -->
#   <arg name="pose" default="0 0 0 0 0 0 1" /> <!-- Initial pose (sim only)   -->

#   <!-- Path to the bag file -->
#   <arg name="bag" default="" />

#   <!-- It doesn't matter that the calling launch file (granite, mtff, etc.) sets these
#        environment variables in the same way. They will be compeletely overridden here. -->
#   <!-- Override the robot and world environment variables all the time. The -->
#   <!-- environment variables are the default if they are set. So in this -->
#   <!-- case we are overriding the environment variables with themselves. -->
#   <!-- Roslaunch arguments override the environment variable which is what -->
#   <!-- this will do. -->
#   <env name="ASTROBEE_ROBOT" value="$(arg robot)" />
#   <env name="ASTROBEE_WORLD" value="$(arg world)" />
#   <env if="$(eval optenv('ASTROBEE_CONFIG_DIR','')=='')"
#        name="ASTROBEE_CONFIG_DIR" value="$(find astrobee)/config" />
#   <env if="$(eval optenv('ASTROBEE_RESOURCE_DIR','')=='')"
#        name="ASTROBEE_RESOURCE_DIR" value="$(find astrobee)/resources" />
#   <env if="$(eval optenv('ROSCONSOLE_CONFIG_FILE','')=='')"
#        name="ROSCONSOLE_CONFIG_FILE" value="$(find astrobee)/resources/logging.config"/>

#   <!-- Launch the platform on its own namespace -->
#   <group ns="/$(arg ns)">

#     <!-- Set the TF prefix, create a robot description and joint state publisher -->
#     <param name="robot_description"
#            command='$(find xacro)/xacro --inorder $(find description)/urdf/model.urdf.xacro world:="$(arg world)" top_aft:="$(arg top_aft)" bot_aft:="$(arg bot_aft)" bot_front:="$(arg bot_front)" ns:="_$(arg ns)" prefix:="$(arg ns)/"'/>
#     <node pkg="robot_state_publisher" type="robot_state_publisher"
#           name="astrobee_state_publisher" />

#     <!-- If we need to load synthetic drivers (we are not running on a real robot) -->
#     <!-- TODO(asymingt) - pass nodes, spurn and extra into gazebo                  -->
#     <include unless="$(arg drivers)"
#            file="$(find astrobee)/launch/controller/synthetic.launch">
#       <arg name="world" value="$(arg world)" />   <!-- World name                -->
#       <arg name="ns" value="$(arg ns)" />         <!-- Robot namespace           -->
#       <arg name="sim" value="$(arg sim)" />       <!-- SIM IP address            -->
#       <arg name="pose" value="$(arg pose)" />     <!-- Initial pose (sim only)   -->
#       <arg name="bag" value="$(arg bag)" />       <!-- Bag to replay             -->
#     </include>

#     <!-- LLP -->
#     <group unless="$(eval arg('llp')=='disabled')">

#       <!-- Connect and update environment variables if required -->
#       <machine unless="$(eval arg('llp')=='local')" timeout="10"
#                name="llp" address="$(arg llp)" user="astrobee" password="astrobee"
#                env-loader="/opt/astrobee/env_wrapper.sh" default="true">
#       </machine>
#       <env unless="$(eval arg('llp')=='local')"
#            name="ROS_HOSTNAME" value="$(arg llp)" />

#       <!-- Update the environment variables relating to absolute paths -->
#       <env unless="$(eval arg('llp')=='local')"
#            name="ASTROBEE_ROBOT" value="$(arg robot)" />
#       <env unless="$(eval arg('llp')=='local')"
#            name="ASTROBEE_WORLD" value="$(arg world)" />
#       <env unless="$(eval arg('llp')=='local')"
#            name="ASTROBEE_CONFIG_DIR" value="/opt/astrobee/config" />
#       <env unless="$(eval arg('llp')=='local')"
#            name="ASTROBEE_RESOURCE_DIR" value="/res" />
#       <env unless="$(eval arg('llp')=='local')"
#            name="ROSCONSOLE_CONFIG_FILE" value="/res/logging.config"/>

#       <!-- If we have specified a bag, then play it back into flight software-->
#       <include file="$(find astrobee)/launch/robot/LLP.launch" >
#         <arg name="drivers" value="$(arg drivers)"/>
#         <arg name="spurn" value="$(arg spurn)" />      <!-- Prevent node         -->
#         <arg name="nodes" value="$(arg nodes)" />      <!-- Launch node group    -->
#         <arg name="extra" value="$(arg extra)" />      <!-- Inject extra nodes   -->
#         <arg name="debug" value="$(arg debug)" />      <!-- Debug node group     -->
#         <arg name="output" value="$(arg output)" />
#         <arg name="gtloc" value="$(arg gtloc)" />
#       </include>

#       <!-- If we have specified a bag, then play it back into flight software-->
#       <!-- node pkg="astrobee" type="check_env.sh" name="llp_check_env" output="screen" -->

#     </group>

#     <!-- MLP -->
#     <group unless="$(eval arg('mlp')=='disabled')">

#       <!-- Connect and update environment variables if required -->
#       <machine unless="$(eval arg('mlp')=='local')" timeout="10"
#                name="mlp" address="$(arg mlp)" user="astrobee" password="astrobee"
#                env-loader="/opt/astrobee/env_wrapper.sh" default="true">
#       </machine>
#       <env unless="$(eval arg('mlp')=='local')"
#            name="ROS_HOSTNAME" value="$(arg mlp)" />

#       <!-- Update the environment variables relating to absolute paths -->
#       <env unless="$(eval arg('mlp')=='local')"
#            name="ASTROBEE_ROBOT" value="$(arg robot)" />
#       <env unless="$(eval arg('mlp')=='local')"
#            name="ASTROBEE_WORLD" value="$(arg world)" />
#       <env unless="$(eval arg('mlp')=='local')"
#            name="ASTROBEE_CONFIG_DIR" value="/opt/astrobee/config" />
#       <env unless="$(eval arg('mlp')=='local')"
#            name="ASTROBEE_RESOURCE_DIR" value="/res" />
#       <env unless="$(eval arg('mlp')=='local')"
#            name="ROSCONSOLE_CONFIG_FILE" value="/res/logging.config"/>

#       <!-- If we have specified a bag, then play it back into flight software-->
#       <include file="$(find astrobee)/launch/robot/MLP.launch" >
#         <arg name="drivers" value="$(arg drivers)"/>
#         <arg name="dds" value="$(arg dds)"/>
#         <arg name="spurn" value="$(arg spurn)" />      <!-- Prevent node         -->
#         <arg name="nodes" value="$(arg nodes)" />      <!-- Launch node group    -->
#         <arg name="extra" value="$(arg extra)" />      <!-- Inject extra nodes   -->
#         <arg name="debug" value="$(arg debug)" />      <!-- Debug node group     -->
#         <arg name="output" value="$(arg output)" />
#         <arg name="gtloc" value="$(arg gtloc)" />
#       </include>

#       <!-- If we have specified a bag, then play it back into flight software-->
#       <!-- node pkg="astrobee" type="check_env.sh" name="mlp_check_env" output="screen" -->

#     </group>

#   </group>

# </launch>
