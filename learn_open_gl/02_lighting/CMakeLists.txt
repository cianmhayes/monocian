cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

add_executable(lighting main.cc)
target_link_libraries(
    lighting
    glad
    glfw
    open_gl
    opengl32
    stb_impl)
target_include_directories(
    lighting PUBLIC
    ../../src
    ../../third_party/glad/include
    ../../third_party/glfw/include
    ../../third_party/glm
    ../../third_party/stb)