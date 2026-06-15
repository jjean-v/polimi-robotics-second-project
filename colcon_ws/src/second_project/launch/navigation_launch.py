import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    pkg_share = get_package_share_directory('second_project')
    nav2_bringup_dir = get_package_share_directory('nav2_bringup')

    world_file = os.path.join(pkg_share, 'world', 'project.world')
    map_yaml = os.path.join(pkg_share, 'map', 'my_map.yaml')
    nav2_params = os.path.join(pkg_share, 'config', 'nav2_params_stage.yaml')
    rviz_config = os.path.join(pkg_share, 'config', 'rviz', 'nav_rviz_config.rviz') 

    navigation_launch = os.path.join(nav2_bringup_dir, 'launch', 'navigation_launch.py')
    use_sim_time = LaunchConfiguration('use_sim_time', default='true')

    stageros_node = Node(
        package='stage_ros2_stageros',
        executable='stageros',
        name='stageros',
        arguments=[world_file],
        parameters=[{'use_sim_time': use_sim_time}]
    )

    nav2_navigation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(navigation_launch),
        launch_arguments={
            'namespace': '',
            'use_sim_time': LaunchConfiguration('use_sim_time'),
            'autostart': LaunchConfiguration('autostart'),
            'params_file': LaunchConfiguration('nav2_params_file'),
            'use_composition': 'False',
            'use_respawn': 'False',
            'log_level': LaunchConfiguration('log_level'),
        }.items(),
    )

    map_server = Node(
        package='nav2_map_server',
        executable='map_server',
        name='map_server',
        output='screen',
        parameters=[
            LaunchConfiguration('nav2_params_file'),
            {
                'use_sim_time': LaunchConfiguration('use_sim_time'),
                'yaml_filename': LaunchConfiguration('map'),
            },
        ],
        remappings=[('/tf', 'tf'), ('/tf_static', 'tf_static')],
    )

    amcl = Node(
        package='nav2_amcl',
        executable='amcl',
        name='amcl',
        output='screen',
        parameters=[
            LaunchConfiguration('nav2_params_file'),
            {
                'use_sim_time': LaunchConfiguration('use_sim_time'),
                'scan_topic': LaunchConfiguration('scan_topic'),
                'base_frame_id': LaunchConfiguration('base_frame'),
                'odom_frame_id': LaunchConfiguration('odom_frame'),
                'global_frame_id': LaunchConfiguration('map_frame'),
                'set_initial_pose': ParameterValue(LaunchConfiguration('set_initial_pose'), value_type=bool),
                'always_reset_initial_pose': ParameterValue(LaunchConfiguration('set_initial_pose'), value_type=bool),
                'initial_pose': {
                    'x': ParameterValue(LaunchConfiguration('initial_x'), value_type=float),
                    'y': ParameterValue(LaunchConfiguration('initial_y'), value_type=float),
                    'z': 0.0,
                    'yaw': ParameterValue(LaunchConfiguration('initial_yaw'), value_type=float),
                },
            },
        ],
        remappings=[('/tf', 'tf'), ('/tf_static', 'tf_static')],
    )

    lifecycle_manager_localization = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager_localization',
        output='screen',
        parameters=[{
            'use_sim_time': LaunchConfiguration('use_sim_time'),
            'autostart': LaunchConfiguration('autostart'),
            'node_names': ['map_server', 'amcl'],
        }],
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config],
        parameters=[{'use_sim_time': use_sim_time}]
    )

    return LaunchDescription([
        DeclareLaunchArgument('use_sim_time', default_value='true', description='Use the simulator /clock.'),
        DeclareLaunchArgument('autostart', default_value='true', description='Automatically configure and activate Nav2 lifecycle nodes.'),
        DeclareLaunchArgument('rviz', default_value='true', description='Start RViz.'),
        DeclareLaunchArgument('map', default_value=map_yaml, description='Occupancy map YAML file for map_server. Defaults to maps/stage_maze.yaml inside this package.'),
        DeclareLaunchArgument('scan_topic', default_value='/base_scan', description='Stage LaserScan topic.'),
        DeclareLaunchArgument('odom_frame', default_value='odom', description='Odometry frame.'),
        DeclareLaunchArgument('map_frame', default_value='map', description='Global map frame.'),
        DeclareLaunchArgument('base_frame', default_value='base_footprint', description='Robot base frame.'),
        DeclareLaunchArgument('set_initial_pose', default_value='true', description='Let AMCL initialize from initial_x, initial_y, initial_yaw. Set false to use RViz 2D Pose Estimate.'),
        DeclareLaunchArgument('initial_x', default_value='8.6', description='Initial robot x in the map frame.'),
        DeclareLaunchArgument('initial_y', default_value='-4.1', description='Initial robot y in the map frame.'),
        DeclareLaunchArgument('initial_yaw', default_value='0.0', description='Initial robot yaw in radians in the map frame.'),
        DeclareLaunchArgument('nav2_params_file', default_value=nav2_params, description='Nav2 parameter file.'),
        DeclareLaunchArgument('rviz_config', default_value=rviz_config, description='RViz config.'),
        DeclareLaunchArgument('log_level', default_value='info', description='Logging level.'),
        stageros_node,
        map_server,
        amcl,
        lifecycle_manager_localization,
        nav2_navigation,
        rviz_node
    ])