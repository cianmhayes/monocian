#include <base/graph.h>

#include <gtest/gtest.h>

TEST(GraphTest, MakeGrid) {
  std::vector<int32_t> grid = {0, 1, 0, 2, 0, -1, 1, 1, 1};
  std::unique_ptr<base::Graph> g = base::Graph::MakeGridGraph<int32_t>(
      grid, 3, [](int32_t start, int32_t end) {
        return static_cast<float>(abs(end - start));
      });
  EXPECT_TRUE(g->ContainsNode(0));
  EXPECT_TRUE(g->ContainsNode(8));
  EXPECT_FALSE(g->ContainsNode(9));
  EXPECT_NEAR(g->GetMinDistance().weight, 0.0, 0.001);
  EXPECT_NEAR(g->GetMaxDistance().weight, 2.0, 0.001);
}

TEST(GraphTest, MinimumSpanningTree) {
  std::unordered_set<size_t> keys = {0, 1, 2, 3, 4, 5, 6};
  std::vector<base::NodeEdge> edges = {
      {0, 1, 7.0},
      {0, 3, 5.0},
      {1, 2, 8.0},
      {1, 3, 9.0},
      {1, 4, 7.0},
      {2, 4, 5.0},
      {3, 4, 15.0},
      {3, 5, 6.0},
      {4, 5, 8.0},
      {4, 6, 9.0},
      {5, 6, 11.0}
    };
  auto g = std::make_unique<base::Graph>(
    std::move(keys),
    std::move(edges)
    );
  auto spanning_tree = g->GetMinimumSpanningTree();
  auto spanning_tree_edges = spanning_tree->GetEdges();
  EXPECT_EQ(spanning_tree_edges.size(), 6);
}