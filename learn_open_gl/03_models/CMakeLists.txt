cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

add_executable(models main.cc)
target_link_libraries(
    models
    assimp::assimp
    glad
    glfw
    open_gl
    opengl32
    stb_impl)
target_include_directories(
    getting_started PUBLIC
    ../../src
    ../../third_party/glad/include
    ../../third_party/glfw/include
    ../../third_party/glm
    ../../third_party/stb)