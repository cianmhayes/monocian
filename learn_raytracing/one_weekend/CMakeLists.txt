cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(one_weekend)

add_executable(one_weekend main.cc)

target_link_libraries(
    one_weekend
    mc_rt
    stb_impl)