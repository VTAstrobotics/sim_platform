from launch import LaunchDescription
from launch.actions import TimerAction
from launch_ros.actions import Node
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    package_share_dir = get_package_share_directory('ukf_launch')
    ukf_config_file = os.path.join(package_share_dir, 'config', 'ukf_config.yaml')
    navsat_config = os.path.join(package_share_dir, 'config', 'navsat_config.yaml')

    ukf_node = Node(
        package='robot_localization',
        executable='ukf_node',
        name='ukf_node',
        output='screen',
        parameters=[ukf_config_file]
    )

    navsat_node = Node(
        package='robot_localization',
        executable='navsat_transform_node',
        name='navsat_transform',
        output='screen',
        parameters=[navsat_config],
        remappings=[
            ('gps/fix', 'gps/data'),
            ('odometry/filtered', 'odometry/filtered'),
            ('imu/data', 'imu/data'),
        ]
    )

    # Delay ukf_node by 5 seconds
    delayed_ukf = TimerAction(
        period=20.0,  # seconds
        actions=[ukf_node, navsat_node]
    )

    return LaunchDescription([
        delayed_ukf,
    ])
