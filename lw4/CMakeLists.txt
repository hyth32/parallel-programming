cmake_minimum_required(VERSION 3.27)
project(lw4)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(lw4 bitmap_image.hpp lw4.cpp)
