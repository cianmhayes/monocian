cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

add_library(
  mc_base
  graph.cc
  storage/broadcast_writer.cc
  storage/file_writer.cc
)

target_include_directories(mc_base PUBLIC ..)