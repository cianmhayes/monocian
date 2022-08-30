cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(monocian)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory(learn_open_gl)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(third_party)
add_subdirectory(tools)