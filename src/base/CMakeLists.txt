cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

add_library(
  base
  graph.cc
)

target_compile_features(base PRIVATE cxx_std_17)

target_include_directories(base PUBLIC ..)