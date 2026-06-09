cmake_minimum_required(VERSION 3.5)
project(second_project)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(ament_index_cpp REQUIRED)

add_executable(goal_publisher src/goal_publisher.cpp)
ament_target_dependencies(goal_publisher 
  rclcpp rclcpp_action nav2_msgs geometry_msgs tf2 tf2_geometry_msgs ament_index_cpp
)

# Install new folders for the simulation, goals, and configs
install(DIRECTORY config launch world csv map
  DESTINATION share/${PROJECT_NAME}
)

install(TARGETS goal_publisher
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()