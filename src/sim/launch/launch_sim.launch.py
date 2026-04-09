import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource

from launch_ros.actions import Node


def generate_launch_description():

    package_name = 'sim'  # <--- CHANGE ME

    world_file_name = 'arena.sdf'

    world_path = os.path.join(
        get_package_share_directory(package_name),
        'worlds',
        world_file_name
    )

    # Robot State Publisher
    rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory(package_name),
                'launch',
                'rsp.launch.py'
            )
        ),
        launch_arguments={'use_sim_time': 'true'}.items()
    )

    # Launch Gazebo Ignition (ros_gz_sim)
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('ros_gz_sim'),
                'launch',
                'gz_sim.launch.py'   # <-- changed from gazebo.launch.py
            )
        ),
        launch_arguments={'gz_args': f'-r {world_path}'}.items()  # <-- important
    )

    # Spawn robot into Ignition
    spawn_entity = Node(
        package='ros_gz_sim',
        executable='create',   # <-- changed from spawn_entity.py
        arguments=[
            '-topic', 'robot_description',
            '-name', 'my_bot'   # <-- changed from -entity
        ],
        output='screen'
    )

    return LaunchDescription([
        rsp,
        gazebo,
        spawn_entity,
    ])