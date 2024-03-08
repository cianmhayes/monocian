cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

add_library(
  mc_rt
  ray.cc
  vec3.cc
)

target_include_directories(mc_rt PUBLIC ..)