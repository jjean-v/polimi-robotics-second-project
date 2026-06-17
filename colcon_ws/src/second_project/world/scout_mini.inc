define kinect ranger (
  sensor ( pose [ 0.0 0.0 0.1 0.0 ] size [0.1 0.1 0.1 ] range [0 8.0] fov 360.0 samples 720 )
  color "black" size [ 0.1 0.1 0.1 ]
)

define scout_mini position (
  pose [ 0.0 0.0 0.0 0.0 ]
  localization "odom"

  # Injecting realistic noise into X, Y, Z, and Theta odometry 
  # X/Y: 3% error | Z: 0% (2D simulation) | Theta: 5% error
  odom_error [ 0.03 0.03 0.0 0.05 ]

  # Approximate Scout Mini dimensions
  size [ 0.62 0.58 0.25 ]  
  
  origin [ 0.0 0.0 0.0 0.0 ]
  gui_nose 1
  drive "omni"
  color "red"
  kinect()
)