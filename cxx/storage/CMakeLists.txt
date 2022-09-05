cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

add_library(
  mc_storage
  broadcast_writer.cc
  buffered_blob_writer.cc
  file_writer.cc
)

target_include_directories(mc_storage PUBLIC ..)