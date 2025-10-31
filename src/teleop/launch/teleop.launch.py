import os

from ament_index_python.packages import get_package_share_directory


from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node

def generate_launch_description():

    spawn_distributor = Node(package="teleop",            
        executable="teleop_node",
        name="teleop_node",
        parameters=[{'use_sim_time': True}]
    )

    spawn_joy = Node(package = "joy",
    executable = "joy_node",
    name = "joy_node",
    parameters=[{'use_sim_time': True}]
    )

    return LaunchDescription([
        spawn_distributor,
        spawn_joy
    ])

