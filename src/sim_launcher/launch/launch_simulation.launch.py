from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
import os
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node

from launch.conditions import IfCondition, UnlessCondition

def generate_launch_description():
    # Locate the other packages' share directories
    sim_share = get_package_share_directory('sim')
    teleop_share = get_package_share_directory('teleop')
    ukf_share = get_package_share_directory('ukf_launch')
    nav2_bringup_share = get_package_share_directory('nav2_bringup')
    our_nav_shar = get_package_share_directory('navigation')
    nav2_params = os.path.join(our_nav_shar, 'config', 'nav2_params.yaml')


    sim_launch = os.path.join(sim_share, 'launch', 'launch_sim.launch.py')
    teleop_launch = os.path.join(teleop_share, 'launch', 'teleop.launch.py')
    ukf_launch = os.path.join(ukf_share, 'launch', 'ukf.launch.py')
    nav2_launch = os.path.join(nav2_bringup_share, 'launch', 'navigation_launch.py')
    
    parameters={
          'frame_id':'base_link',
          'use_sim_time':True,
          'subscribe_depth':True,
          'use_action_for_goal':True,
          'Reg/Force3DoF':'true',
          'Grid/RayTracing':'true', # Fill empty space
          'Grid/3D':'false', # Use 2D occupancy
          'Grid/RangeMax':'3',
          'Grid/NormalsSegmentation':'false', # Use passthrough filter to detect obstacles
          'Grid/MaxGroundHeight':'0.05', # All points above 5 cm are obstacles
          'Grid/MaxObstacleHeight':'0.4',  # All points over 1 meter are ignored
          'wait_for_transform_duration': 1,
          'Optimizer/GravitySigma':'0', # Disable imu constraints (we are already in 2D)
          'Grid/FrameId':'map'
    }

    remappings=[
            ('rgb/image', '/camera/image_raw'),
            ('rgb/camera_info', '/camera/camera_info'),
            ('depth/image', '/camera/depth/image_raw'),
            ('odom', '/odometry/filtered'), 
          ]
        
        # SLAM mode:
    slam = Node(
            package='rtabmap_slam', executable='rtabmap', output='screen',
            parameters=[parameters],
            remappings=remappings,
            arguments=['-d']) # This will delete the previous database (~/.ros/rtabmap.db)

    

    return LaunchDescription([
        IncludeLaunchDescription(PythonLaunchDescriptionSource(sim_launch)),
        IncludeLaunchDescription(PythonLaunchDescriptionSource(teleop_launch)),
        IncludeLaunchDescription(PythonLaunchDescriptionSource(ukf_launch)),
        IncludeLaunchDescription(PythonLaunchDescriptionSource(nav2_launch), 
                                 launch_arguments={
                                    'use_sim_time': 'true',
                                    'params_file': nav2_params,
                                }.items()
                            ),

        slam

        
    ])

