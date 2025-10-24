# This project allows you to drive a robot in simulation using a joystick controller with ROS 2 Humble and Gazebo Classic.

## Requirements

- ROS 2 Humble
- Gazebo Classic
- The following packages: 
  ```
  sudo apt install \
	  ros-humble-gazebo-ros-pkgs \
	  ros-humble-joy \
	  ros-humble-teleop-twist-joy \
	  ros-humble-xacro \
	  ros-humble-joint-state-publisher \
	  ros-humble-robot-state-publisher \
	  ros-humble-rviz2
  ```
## Setup & Usage

- Connect your joystick controller to your computer
- Run the bringup script:
   ```
   ./startup/bringup_robot.sh
   ```
- You should now see Gazebo open and spawn our robot model. You can now drive the robot around the environment with the left joystick on your controller.

## Other Useful Commands

- If you want to see the data in another terminal, run the source command
   ```
   source /usr/share/gazebo/setup.sh
   ```
- Then you can echo the topic data
   ```
   ros2 topic list
   ```
   ```
   ros2 topic echo [topic name]
   ```
## Future Use

We are hoping to use this platform as a way to develop and test our high-level autonomy.

