The normal behavior of the package is to launch the slam algorithm with

'''
ros2 launch second_project second_project_launch.py
'''

Then the navigation is launched with 

'''
ros2 launch second_project navigation_launch.py
'''

We took the template from the ROS lectures but had to modify a few points:
- switching from differential drive to omnidirectionnal made us modify the speeds and velocities in the y axis.
- using a rectangel robot instead of a round one made us use footprints instead of radius.

Looking on the website of the AgileX scout mini, https://global.agilex.ai/products/scout-mini, we took the official dimensions.
However, regarding the maximum speed we had to tune it down because of the refreshing frequency of the controller and the thin corridors.

It is intended to wait 5 seconds before starting the goal_publisher node, to make sure that everything related to stage and rviz2 started correctly. Still, a double check is performed through "action_server_is_ready()" method.

The navigation being non-deterministic it happens that the robot does not find its way, from our tests it has a ~50% pass rate on the (-4.7,2.7,1.347) goal.
The fastest run was performed in ~120 seconds, and a screenshot proving that it can perform the complete task is available in the package : robotics_all_task_complete_screenshot.png