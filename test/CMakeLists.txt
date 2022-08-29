cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

enable_testing()

add_executable(
  unit_tests
  base/graph_test.cc
  base/merge_test.cc
  selective_search/selective_search_test.cc
)

target_link_libraries(
  unit_tests
  GTest::gtest_main
  base
  selective_search
)

include(GoogleTest)
gtest_discover_tests(unit_tests)