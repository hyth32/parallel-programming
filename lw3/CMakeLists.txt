cmake_minimum_required(VERSION 3.27)
project(lw3)

set(CMAKE_CXX_STANDARD 14)

add_executable(lw3 lw3.cpp)

target_link_libraries(lw3 winmm)