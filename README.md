<head>
  <h1>
    Astrobee Robot Software
  </h1>
</head>

<body>
<details open>
  <summary><h2>About</h2></summary>
<img src="doc/images/astrobee.png" srcset="../images/astrobee.png 1x"
  title="Astrobee" align="right" style="display: inline"/>
  <ul><li>
Three NASA <a href="https://nasa.gov/astrobee/" target="_blank">Astrobee free-flying robots</a> have been operating inside
the International Space Station (ISS) since 2019. This <a href="https://github.com/nasa/astrobee" target="_blank">`astrobee`</a> repository contains source code for the Astrobee Robot Software, consisting of
the flight software that runs onboard the Astrobee robots, a software simulator, and supporting tools, primarily written in C++.
</li>
<li>Astrobee's flight software performs vision-based localization,
provides autonomous navigation, docking and perching, manages various sensors
and actuators, and supports human-robot interaction with co-located astronauts via screen-based displays, light
signaling, and sound. The flight software is hosted on each Astrobee's three internal single board computers and
uses the open-source <a href="https://www.ros.org/" target="_blank">Robot Operating System (ROS)</a> framework as message-passing
middleware.  It provides a high-level <a href="https://nasa.github.io/astrobee/html/command_dictionary.html" target="_blank">Astrobee Command API</a> for controlling the robot and has multiple operating modes. It can execute a plan (command sequence), individual operator commands (teleoperation), or commands from guest science code running onboard Astrobee.</li>

<li>The Astrobee Robot Software simulator, built using <a href="https://www.ros.org/" target="_blank">ROS</a> and <a href="http://gazebosim.org/" target="_blank">Gazebo</a>, enables the flight software to be evaluated without the need for robot hardware. The supporting tools include a tool that processes ISS imagery to <a href="https://nasa.github.io/astrobee/html/sparsemapping.html" target="_blank">build maps for Astrobee localization</a>, along with many others.</li>

<li>Released separately, the Astrobee ground data system (GDS) includes Astrobee control station software that communicates with Astrobee flight software via the Data Distribution Service (DDS) network protocol over the ISS Ku-IP space-to-ground link. The control station is written primarily in Java using the Eclipse RCP framework. Source code is in the <a href="https://github.com/nasa/astrobee_gds" target="_blank">`astrobee_gds`</a> repository, or you can download the <a href="https://software.nasa.gov/software/ARC-17994-1B" target="_blank">binary release</a> Together.</li>

<li>the Astrobee Robot and Ground Software were the NASA Software of the Year Award Runner-Up in 2020!</li>

<li>The Astrobee Robot Software remains a work in progress. Please consult the
<a href="https://nasa.github.io/astrobee/html/md_RELEASE.html" target="_blank">release notes</a> for the current list of features and limitations.</li>
  </ul>
  </details>
  
<details open>
  <summary><h2>Usage</h2></summary>
  <div><ul>
    <li>
<a href="https://nasa.github.io/astrobee/html/md_INSTALL.html" target="_blank">Instructions</a> on installing and using the Astrobee Robot Software.</li>
    </ul></div>
  </details>

<details open>
  <summary><h2>Contributing</h2></summary>
<div><ul><li>The Astrobee Robot Software is open source, and we welcome contributions
from the public. Please submit pull requests to the <a href="https://github.com/nasa/astrobee/tree/develop" target="_blank">`develop`</a> branch. The code must follow the <a href="https://nasa.github.io/astrobee/html/astrobee-code-style.html" target="_blank">Astrobee code style</a>.</li>
<li>For us to merge any pull requests, we must request that contributors sign and submit a
<a href="https://www.nasa.gov/sites/default/files/atoms/files/astrobee_individual_contributor_license_agreement.pdf" target="_blank">Contributor License Agreement</a>
  due to NASA legal requirements. Thank you for your understanding.</li></ul></div>
</details>
  
<details open>
  <summary><h2>Documentation</h2></summary>
  <div><ul>
<li>Extensive <a href="https://nasa.github.io/astrobee/documentation.html" target="_blank">Extensive</a> documentation is auto-generated from the contents of this repository.</li>

<li><a href="https://github.com/albee/a-brief-guide-to-astrobee/raw/master/a_brief_guide_to_astrobee_latest.pdf" target="_blank">"A Brief Guide to Astrobee’s Flight Software is a good tutorial"</a>, with a particular emphasis on the advanced topic of modifying Astrobee's flight software to enable Guidance, Navigation, & Control (GN&C) research. (Note that most guest science can be implemented as an app that uses the <a href="https://nasa.github.io/astrobee/html/command_dictionary.html" target="_blank">Astrobee Command API</a> without modifying the flight software.)</li>

<li>The <a href="https://github.com/Pedro-Roque/astrobee_ros_demo" target="_blank">"Astrobee ROS Demo"</a> package provides a C++/Python interface to implement custom control methods in ROS.</li>

<li>For more information, read <a href="https://www.nasa.gov/content/research-publications-0" target="_blank">Astrobee-related publications</a>.</li>
<li>Learning about the Astrobee <a href="https://www.nasa.gov/sites/default/files/atoms/files/bualat_spaceops_2018_paper.pdf" target="_blank">platform</a>,<a href="https://www.nasa.gov/sites/default/files/atoms/files/fluckiger2018astrobee.pdf" target="_blank"> software</a>, and <a href="https://www.nasa.gov/sites/default/files/atoms/files/coltin2016localization.pdf" target="_blank">localization</a> are good starting points.</li>
  </ul></div>
</details>
  
<details open>
  <summary><h2>Guest Science</h2></summary>
  <div><ul>
<li>The ISS Astrobee Facility maintains a collection of <a href="https://www.nasa.gov/content/guest-science-resources" target="_blank">resources for guest scientists</a>  interested in conducting research with Astrobee, including the <a href="https://www.nasa.gov/sites/default/files/atoms/files/irg-ff029-astrobee-guest-science-guide.pdf" target="_blank">Astrobee Guest Science Guide</a> overview.</li>
<li>
Guest science code that runs onboard Astrobee is usually hosted as an Android Java app running on Astrobee's high-level processor (HLP).</li>
<li>Guest science apps can use the <a href="https://nasa.github.io/astrobee/html/command_dictionary.html" target="_blank">Astrobee Command API</a> through its Java bindings. The <a href="https://github.com/nasa/astrobee_android" target="_blank">`astrobee_android`</a> repository contains source code for Astrobee services that run on the HLP, example guest science apps, as well as a <a href="https://github.com/nasa/astrobee_android/blob/master/guest_science_readme.md" target="_blank">Guest Science Readme</a> and <a href="https://github.com/nasa/astrobee_android/blob/master/gs_developer_guide.md" target="_blank">Guest Science Developer Guide</a> focused on HLP guest science apps.</li>
    </ul></div>
    </details>
  
<details open>
  <summary><h2>lisense</h2></summary> 
  <div><ul>
    <li>Copyright (c) 2017, United States Government, as represented by the
Administrator of the National Aeronautics and Space Administration.
      All rights reserved.</li>

<li>The Astrobee platform is licensed under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance with the License. You
      may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.</li>

<li>Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
  specific language governing permissions and limitations under the License.</li>
    </ul></div>
  </details>
</body>
