
#include "selective_search/selective_search.h"

#include <iostream>

#include <gtest/gtest.h>

TEST(SelectiveSearchTest, IntSearch) {
  std::vector<int32_t> grid = {
      0, 1, 1, 0, 2, 2, 0, 
      0, 1, 1, 0, 2, 2, 0,
      0, 0, 0, 0, 2, 2, 0,
      0, 0, 3, 0, 2, 2, 0,
      0, 3, 3, 0, 2, 2, 0,
      3, 3, 3, 3, 2, 2, 0,
  };
  auto g = base::Graph::MakeGridGraph<int32_t>(
      grid, 7, [](const int32_t& start, const int32_t& end) {
        return start == end ? 0.0f : 1.0f;
      });
  auto components = selective_search::SelectiveSearch(
      *g.get(),
      [](const selective_search::Component& c) { return 0.5f / (c.Size() + 1); });

  EXPECT_EQ(components.size(), 5);
}