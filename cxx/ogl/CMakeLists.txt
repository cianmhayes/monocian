cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

add_library(
    mc_ogl
    camera.cc
    full_screen_video.cc
    shader.cc
    simple_mesh.cc
    text_overlay_renderer.cc
    window.cc)

target_include_directories(
    mc_ogl PUBLIC
    ../../third_party/glad/include
    ../../third_party/glfw/include
    ../../third_party/glm
    ../../third_party/stb)