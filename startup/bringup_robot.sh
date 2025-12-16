#!/bin/bash
source /opt/ros/humble/setup.bash

colcon build
source install/setup.bash
source /usr/share/gazebo/setup.sh
GAZEBO_RESOURCE_PATH=$GAZEBO_RESOURCE_PATH:$(ros2 pkg prefix sim)/share/sim/worlds
ros2 param set /rviz use_sim_time true
ros2 launch sim_launcher launch_simulation.launch.py

