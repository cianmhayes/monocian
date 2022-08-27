cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

add_library(
  stb_impl
  stb_impl.cc
)

target_include_directories(stb_impl PUBLIC ../../third_party/stb)