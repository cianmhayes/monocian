cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

add_library(
    open_gl
    camera.cc
    full_screen_video.cc
    shader.cc
    window.cc)

target_include_directories(
    open_gl PUBLIC
    ../../third_party/glad/include
    ../../third_party/glfw/include
    ../../third_party/glm
    ../../third_party/stb)