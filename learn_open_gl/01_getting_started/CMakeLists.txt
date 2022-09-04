cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

add_executable(getting_started main.cc)
target_link_libraries(
    getting_started
    glad
    glfw
    mc_ogl
    opengl32
    stb_impl)
target_include_directories(
    getting_started PUBLIC
    ../../cxx
    ../../third_party/glad/include
    ../../third_party/glfw/include
    ../../third_party/glm
    ../../third_party/stb)