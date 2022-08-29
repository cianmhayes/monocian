cmake_minimum_required(VERSION 3.22)


find_package(azure-storage-blobs-cpp CONFIG REQUIRED)
find_package(FFMPEG COMPONENTS AVCODEC AVFORMAT AVUTIL AVDEVICE REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(OpenGL REQUIRED) # required to link realsense2
find_package(realsense2 CONFIG REQUIRED)

add_executable(realsense_capture
               main.cc
               file_writer.cc
               broadcast_writer.cc
               buffered_blob_writer.cc
               video_encoder.cc
               video_encoding_queue.cc)

target_include_directories(
    realsense_capture PUBLIC
    ../../third_party)

target_link_libraries(realsense_capture
                      glfw
                      ${OPENGL_LIBRARIES}
                      ${FFMPEG_LIBRARIES}
                      Azure::azure-storage-blobs
                      realsense2::realsense2
                      realsense2::realsense-file
                      jsoncpp_lib
                      jsoncpp_object
                      JsonCpp::JsonCpp
                      glog::glog)

