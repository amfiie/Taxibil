srcs = files('main.cpp', 'utils.cpp')
opencv_dep = dependency('opencv4')
fmt_dep = dependency('fmt')

if get_option('calibration')
    add_project_arguments('-DCALIBRATION=1', language:'cpp')
endif
executable('lane_detection', srcs, dependencies: [opencv_dep, fmt_dep], cpp_args: '-std=c++17')


# meson setup build
# ninja -C build
#./build/src/lane_detection