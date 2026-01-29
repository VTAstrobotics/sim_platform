import os

from ament_index_python.packages import get_package_share_directory


from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node

def generate_launch_description():

    spawn_waypoint_translator = Node(package="waypoint_translator",            
        executable="waypoint_translator_node",
        name="waypoint_translator_node",
        parameters=[{'use_sim_time': True}]
    )
    

    return LaunchDescription([
        spawn_waypoint_translator
    ])

