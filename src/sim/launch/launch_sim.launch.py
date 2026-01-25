import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node


def generate_launch_description():
    package_name = "sim"
    world_file_name = "empty.world"

    world_path = os.path.join(
        get_package_share_directory(package_name),
        "worlds",
        world_file_name,
    )

    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory(package_name), "launch", "rsp.launch.py")
        ),
        launch_arguments={"use_sim_time": "true"}.items(),
    )

    # Gazebo Sim / Ignition (ros_gz_sim)
    gazebo_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory("ros_gz_sim"), "launch", "gz_sim.launch.py")
        ),
        # gz_args is passed to `gz sim` (or `ign gazebo` on older distros)
        # Typical flags:
        #   -r : run immediately
        #   -v 4 : verbosity
        launch_arguments={"gz_args": f"-r -v 4 {world_path}"}.items(),
    )

    # Spawn robot into Gazebo Sim
    # This is the analogue of gazebo_ros spawn_entity.py
    spawn_entity = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=[
            "-name", "my_bot",
            "-topic", "robot_description",
        ],
        output="screen",
    )

    # Bridge /clock from Gazebo Sim to ROS (often required for use_sim_time to work reliably)
    clock_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock",
        ],
        output="screen",
    )

    return LaunchDescription([
        rsp,
        gazebo_sim,
        clock_bridge,
        spawn_entity,
    ])
