define kinect ranger (
  sensor ( pose [ 0.0 0.0 0.1 0.0 ] size [0.1 0.1 0.1 ] range [0 8.0] fov 360.0 samples 720 )
  color "black" size [ 0.1 0.1 0.1 ]
)

define scout_mini position (
  pose [ 0.0 0.0 0.0 0.0 ]
  localization "odom"

  # Injecting realistic noise into X, Y, Z, and Theta odometry 
  # X/Y: 0.5% error | Z: 0% (2D simulation) | Theta: 1% error
  odom_error [ 0.005 0.005 0.0 0.01 ]

  # Approximate Scout Mini dimensions from https://global.agilex.ai/products/scout-mini
  size [ 0.612 0.580 0.245 ]  

  origin [ 0.0 0.0 0.0 0.0 ]
  gui_nose 1
  drive "omni"
  color "red"
  kinect()
)