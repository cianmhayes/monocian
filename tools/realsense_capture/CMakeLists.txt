cmake_minimum_required(VERSION 3.22)

add_executable(realsense_capture
               main.cc)

target_include_directories(
    realsense_capture PUBLIC
    ../../cxx
    ../../third_party)

target_link_libraries(realsense_capture
                      glfw
                      opengl32
                      mc_av
                      mc_az
                      mc_base
                      mc_ogl
                      ${FFMPEG_LIBRARIES}
                      Azure::azure-storage-blobs
                      realsense2::realsense2
                      realsense2::realsense-file
                      jsoncpp_lib
                      jsoncpp_object
                      JsonCpp::JsonCpp
                      glog::glog)

