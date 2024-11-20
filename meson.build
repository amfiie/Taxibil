src = files(
    'main.cpp',
    'i2c-master.cpp',
    'pathing.cpp',
    'server.cpp',
)

cmake = import('cmake')

sockpp_proj = cmake.subproject('sockpp')
sockpp_lib = sockpp_proj.dependency('sockpp')
fmt_dep = dependency('fmt')
thread_dep = dependency('threads') 

executable('kommunikationsmodul', src,
           dependencies: [sockpp_lib, fmt_dep, thread_dep],
           cpp_args: '-std=c++20')