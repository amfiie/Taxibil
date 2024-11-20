srcs = files('main.cpp')
opencv_dep = dependency('opencv4')
fmt_dep = dependency('fmt')
threads_dep = dependency('threads')

executable('camera', srcs, dependencies: [opencv_dep, fmt_dep, threads_dep], cpp_args: '-std=c++17')
