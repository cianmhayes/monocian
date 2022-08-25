cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

add_library(
  selective_search
  component.cc
  selective_search.cc
)

target_compile_features(selective_search PRIVATE cxx_std_17)

target_include_directories(selective_search PUBLIC ..)