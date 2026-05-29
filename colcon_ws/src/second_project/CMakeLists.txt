cmake_minimum_required(VERSION 3.5)
project(second_project)

find_package(ament_cmake REQUIRED)

# Installe le dossier launch
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()