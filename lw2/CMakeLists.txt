cmake_minimum_required(VERSION 3.27)
project(lw2)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(lw2
        bitmap_image.hpp
        lw2.cpp)
