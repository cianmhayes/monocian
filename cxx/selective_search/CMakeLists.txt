cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

add_library(
  mc_selective_search
  selective_search.cc
)

target_include_directories(mc_selective_search PUBLIC ..)