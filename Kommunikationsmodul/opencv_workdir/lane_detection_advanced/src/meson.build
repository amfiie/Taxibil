srcs = files('main.cpp', 'trackbars.cpp', 'binarization-utils.cpp', 'perspective_utils.cpp', 'line-utils.cpp', 'maths.cpp')
opencv_dep = dependency('opencv4')
fmt_dep = dependency('fmt')

executable('lane_detection_advanced', srcs, dependencies: [opencv_dep, fmt_dep], cpp_args: ['-std=c++17', '-O2'])