define kinect ranger (
  sensor ( pose [ 0.0 0.0 0.1 0.0 ] size [0.1 0.1 0.1 ] range [0 8.0] fov 360.0 samples 720 )
  color "black" size [ 0.1 0.1 0.1 ]
)

define scout_mini position (
  pose [ 0.0 0.0 0.0 0.0 ]
  localization "odom"
  odom_error [0.0 0.0 0.0 0.0 ]
  size [ 0.62 0.58 0.25 ]  # Approximate Scout Mini dimensions
  origin [ 0.0 0.0 0.0 0.0 ]
  gui_nose 1
  drive "omni"
  color "red"
  kinect()
)