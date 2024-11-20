src = files(
    'main.cpp',
    'pathing.cpp',
    'server.cpp',
    'steering.cpp',
    'i2c-master.cpp',
    'computer_vision/binarization-utils.cpp', 
    'computer_vision/perspective-utils.cpp', 
    'computer_vision/line-utils.cpp', 
    'computer_vision/maths.cpp',
    'computer_vision/pipeline.cpp',
    'computer_vision/computer-vision.cpp',
)

cmake = import('cmake')

sockpp_proj = cmake.subproject('sockpp')
sockpp_lib = sockpp_proj.dependency('sockpp')
fmt_dep = dependency('fmt')
thread_dep = dependency('threads')
opencv_dep = dependency('opencv4')

incdir = include_directories('../../Include')

add_project_arguments('-DCOMMUNICATION_MODULE=1', language:'cpp')

executable('kommunikationsmodul', src, include_directories : incdir,
           dependencies: [sockpp_lib, fmt_dep, thread_dep, opencv_dep],
           cpp_args: ['-std=c++17'])
