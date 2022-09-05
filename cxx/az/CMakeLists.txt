cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

add_library(
  mc_az
  buffered_blob_writer.cc
)

target_include_directories(mc_az PUBLIC ..)