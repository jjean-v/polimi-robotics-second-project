# Polimi Robotics Second Project

The goal of this group project is to, from a bag file, compute the a map of the environment, and then navigate the robot in the environment, using the map. The project is divided into two parts: the first part is to compute the map of the environment, and the second part is to navigate the robot in the environment.

## How to run the project

1. Start the docker container

2. Install the dependencies

```bash
apt install ros-humble-pointcloud-to-laserscan
```

>    Note: to do this you need the root access, so you need to run the docker container with the command:
>   ```bash
>   sudo ./start_as_root.sh
>   ``` 

3. Build the workspace

```bash
colcon build 
source install/setup.bash
```

4. Run the launch file

```bash
ros2 launch second_project second_project_launch.py
```

5. Run the bag file. Due to its size you need to manually download the bag file from the link provided in the project description

6. Use rviz to visualize the map and the laser scan

> - Add **Tf**
> - Add **LaserScan**
> - In the LaserScan settings, select best effort to get the correct frame and topic
> - Select **UVG_odom** as the fixed frame
> - Optionnaly add **CloudPoint**


## Roadmap 

- [x] Convert Point Cloud To Laser Scan
- [] Fine tune the parameters of the node to get a good quality laser scan
- [] Use the laser scan to compute the map of the environment
- [] Use the map to navigate the robot in the environment
