#include <base/merge.h>

#include <base/graph.h>
#include <gtest/gtest.h>

int32_t MergeInts(std::list<int32_t>&& ints) {
  int32_t sum = 0;
  for (auto i : ints) {
    sum += i;
  }
  return sum;
}

TEST(MergeTest, SumAll) {
  std::list<int32_t> values = {1, 2};
  base::Merge<int32_t>(
      values, [](const int32_t& i) { return true; }, MergeInts);
  EXPECT_EQ(values.size(), static_cast<size_t>(1));
  EXPECT_EQ(values.front(), static_cast<size_t>(3));
}

TEST(MergeTest, MultiplePassesOnInt) {
  std::list<int32_t> values = {1, 2, 3, 4, 5};
  base::Merge<int32_t>(
      values, [](const int32_t& i) { return true; },
      MergeInts);
  EXPECT_EQ(values.size(), static_cast<size_t>(4));

  base::Merge<int32_t>(
      values, [](const int32_t& i) { return true; },
      MergeInts);
  EXPECT_EQ(values.size(), static_cast<size_t>(3));

  base::Merge<int32_t>(
      values, [](const int32_t& i) { return true; },
      MergeInts);
  EXPECT_EQ(values.size(), static_cast<size_t>(2));

  base::Merge<int32_t>(
      values, [](const int32_t& i) { return true; },
      MergeInts);
  EXPECT_EQ(values.size(), static_cast<size_t>(1));
  EXPECT_EQ(values.front(), static_cast<size_t>(15));
}

TEST(MergeTest, MergeGraphs) {
  std::list<size_t> keys = {0, 1, 2, 3, 4};
  std::list<std::unique_ptr<base::Graph>> graphs = {};
  graphs.push_back(std::make_unique<base::Graph>(std::unordered_set<size_t>({0, 2, 4}), std::vector<base::NodeEdge>({{0, 2, 1.0}, {2, 4, 1.0}})));
  graphs.push_back(std::make_unique<base::Graph>(std::unordered_set<size_t>({1, 5}), std::vector<base::NodeEdge>({{0, 5, 1.0}})));
  graphs.push_back(std::make_unique<base::Graph>(std::unordered_set<size_t>({3, 6}), std::vector<base::NodeEdge>({{3, 6, 1.0}})));
  base::Merge<std::unique_ptr<base::Graph>>(
    graphs,
    [](const std::unique_ptr<base::Graph>& g){ return g->ContainsNode(0) || g->ContainsNode(1);},
    [](std::list<std::unique_ptr<base::Graph>>&& mergable){
        base::Graph* g1 = mergable.front().get();
        base::Graph* g2 = mergable.back().get();
        base::NodeEdge e = {0, 1, 1.0};
        return std::make_unique<base::Graph>(*g1, *g2, e);
    });
  base::Merge<std::unique_ptr<base::Graph>>(
    graphs,
    [](const std::unique_ptr<base::Graph>& g){ return g->ContainsNode(2) || g->ContainsNode(3);},
    [](std::list<std::unique_ptr<base::Graph>>&& mergable){
        base::Graph* g1 = mergable.front().get();
        base::Graph* g2 = mergable.back().get();
        base::NodeEdge e = {2, 3, 1.0};
        return std::make_unique<base::Graph>(*g1, *g2, e);
    });
}
