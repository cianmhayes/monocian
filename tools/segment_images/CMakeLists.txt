cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

add_executable(segment_images main.cc)

target_include_directories(
    segment_images PUBLIC
    ../../cxx
    ../../third_party/cxxopts/include
    ../../third_party/stb)

target_link_libraries(
  segment_images
  mc_base
  mc_selective_search
  stb_impl
)