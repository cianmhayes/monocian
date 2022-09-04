cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(monocian)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(assimp CONFIG REQUIRED)
find_package(azure-storage-blobs-cpp CONFIG REQUIRED)
find_package(FFMPEG COMPONENTS AVCODEC AVFORMAT AVUTIL AVDEVICE REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(realsense2 CONFIG REQUIRED)

add_subdirectory(learn_open_gl)
add_subdirectory(cxx)
add_subdirectory(test)
add_subdirectory(third_party)
add_subdirectory(tools)