from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # Locate the other packages' share directories
    sim_share = get_package_share_directory('sim')
    teleop_share = get_package_share_directory('teleop')

    sim_launch = os.path.join(sim_share, 'launch', 'launch_sim.launch.py')
    teleop_launch = os.path.join(teleop_share, 'launch', 'teleop.launch.py')

    return LaunchDescription([
        IncludeLaunchDescription(PythonLaunchDescriptionSource(sim_launch)),
        IncludeLaunchDescription(PythonLaunchDescriptionSource(teleop_launch)),
    ])

