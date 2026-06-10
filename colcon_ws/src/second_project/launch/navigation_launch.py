import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    pkg_share = get_package_share_directory('second_project')
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')

    world_file = os.path.join(pkg_share, 'world', 'project.world')
    map_yaml = os.path.join(pkg_share, 'map', 'my_map.yaml')
    nav2_params = os.path.join(pkg_share, 'config', 'nav2_params_stage.yaml')
    rviz_config = os.path.join(pkg_share, 'config', 'rviz', 'mapping_rviz_config.rviz') 

    use_sim_time = LaunchConfiguration('use_sim_time', default='true')

    stageros_node = Node(
        package='stage_ros2_stageros',
        executable='stageros',
        name='stageros',
        arguments=[world_file],
        parameters=[{'use_sim_time': use_sim_time}]
    )

    nav2_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(nav2_bringup_dir, 'launch', 'bringup_launch.py')),
        launch_arguments={
            'use_sim_time': use_sim_time,
            'map': map_yaml,
            'params_file': nav2_params,
            'autostart': 'true'
        }.items()
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config],
        parameters=[{'use_sim_time': use_sim_time}]
    )

    return LaunchDescription([
        stageros_node,
        nav2_launch,
        rviz_node
    ])