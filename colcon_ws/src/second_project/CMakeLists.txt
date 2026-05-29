cmake_minimum_required(VERSION 3.5)
project(second-project)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

ament_auto_add_library(second_project SHARED
  src/pointcloud_to_laserscan_node.cpp)

rclcpp_components_register_node(second_project
  PLUGIN "second_project::PointCloudToLaserScanNode"
  EXECUTABLE second_project_node)

ament_auto_add_executable(dummy_pointcloud_publisher
  src/dummy_pointcloud_publisher.cpp
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_auto_package(
  INSTALL_TO_SHARE
    launch
)
