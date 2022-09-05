cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

add_library(
  mc_av
  video_encoder.cc
  video_encoding_queue.cc
)

target_include_directories(mc_av PUBLIC ..)