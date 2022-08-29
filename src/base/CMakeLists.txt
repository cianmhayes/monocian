cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

add_library(
  base
  graph.cc
)

target_include_directories(base PUBLIC ..)