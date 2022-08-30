cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

include(../../src/resources/CMakeLists.txt)

embed_resources(
    shader_glsl
    shaders/common_vertex.glsl
    shaders/light_source_frag.glsl
    shaders/lit_object_frag.glsl)

embed_resources(
    textures
    assets/container2.png
    assets/container2_specular.png)

add_executable(
    lighting
    main.cc
    ${shader_glsl}
    ${textures})

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