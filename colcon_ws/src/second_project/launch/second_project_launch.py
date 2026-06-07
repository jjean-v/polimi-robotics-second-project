from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    pkg_share = FindPackageShare('second_project')
    default_params = PathJoinSubstitution([pkg_share, 'config', 'mapper_params.yaml']) 
    rviz_config = PathJoinSubstitution([pkg_share, 'config','rviz', 'mapping_rviz_config.rviz']) 
    print("rviz_config: ", rviz_config)

    rviz = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', LaunchConfiguration('rviz_config')],
        condition=IfCondition(LaunchConfiguration('rviz')),
        parameters=[{'use_sim_time': LaunchConfiguration('use_sim_time')}],
    )

    return LaunchDescription([
        DeclareLaunchArgument('rviz', default_value='true', description='Start RViz.'),
        DeclareLaunchArgument(name='scanner', default_value='ugv',description='Namespace for sample topics'),
        DeclareLaunchArgument('slam_mode', default_value='async', description='Use async or sync SLAM Toolbox mapping.'),
        DeclareLaunchArgument('use_sim_time', default_value='true', description='Use the simulator /clock.'),
        DeclareLaunchArgument('scan_topic', default_value='/ugv/scan', description='LaserScan topic already published by the simulator.'),
        DeclareLaunchArgument('odom_frame', default_value='UGV_odom', description='Odometry frame.'),
        DeclareLaunchArgument('map_frame', default_value='map', description='Map frame published by SLAM Toolbox.'),
        DeclareLaunchArgument('base_frame', default_value='UGV_base_link', description='Robot base frame used by SLAM Toolbox.'),
        DeclareLaunchArgument('slam_params_file', default_value=default_params, description='SLAM Toolbox parameter file.'),
        DeclareLaunchArgument('rviz_config', default_value=rviz_config, description='RViz config file.'),

        Node(
            package='pointcloud_to_laserscan', executable='pointcloud_to_laserscan_node',
            remappings=[('cloud_in', [LaunchConfiguration(variable_name='scanner'), '/rslidar_points']),
                        ('scan', [LaunchConfiguration(variable_name='scanner'), '/scan'])],
            parameters=[{
                'target_frame': 'rslidar',
                'transform_tolerance': 0.01,
                'min_height': -0.3,
                'max_height': 0.4,
                'angle_min': -3.14159,
                'angle_max': 3.14159,
                'angle_increment': 0.00175,
                'scan_time': 0.1,
                'range_min': 0.15,
                'range_max': 100.0,
                'use_inf': True,
                'inf_epsilon': 1.0
            }],
            name='pointcloud_to_laserscan'
        ),
        Node(
            package='slam_toolbox',
            executable='async_slam_toolbox_node',
            name='slam_toolbox',
            output='screen',
            parameters=[
                LaunchConfiguration('slam_params_file'),
                {
                    'use_sim_time': LaunchConfiguration('use_sim_time'),
                    'mode': 'mapping',
                    'odom_frame': LaunchConfiguration('odom_frame'),
                    'map_frame': LaunchConfiguration('map_frame'),
                    'base_frame': LaunchConfiguration('base_frame'),
                    'scan_topic': LaunchConfiguration('scan_topic'),
                },
            ],
        ),
        rviz
    ])