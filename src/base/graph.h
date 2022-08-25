#ifndef SRC_BASE_GRAPH_H_
#define SRC_BASE_GRAPH_H_

#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

namespace base {

struct NodeEdge {
 public:
  size_t first;
  size_t second;
  float weight;
};

template <typename TValue>
class WeightFunction {
 public:
  virtual float Compute(const TValue& lhs, const TValue& rhs) const = 0;
};

class Graph {
 public:
  Graph(std::unordered_set<size_t>&& keys, std::vector<NodeEdge>&& edges);
  Graph(const Graph& lhs, const Graph& rhs, NodeEdge edge);
  ~Graph();
  Graph(const Graph&) = delete;
  Graph& operator=(const Graph&) = delete;

  template <typename TValue>
  static std::unique_ptr<Graph> MakeGridGraph(
      const std::vector<TValue>& grid,
      size_t width,
      std::function<float(const TValue&, const TValue&)> diff_function) {
    std::unordered_set<size_t> keys(grid.size());
    for (size_t i = 0; i < grid.size(); i++)
      keys.insert(i);
    std::vector<NodeEdge> edges = {};
    for (size_t i = 0; i < grid.size(); i++) {
      // horizontal
      if ((i+1) % width != 0)
        edges.push_back({i, i + 1, diff_function(grid[i], grid[i + 1])});
      // vertical
      if (i + width < grid.size())
        edges.push_back(
            {i, i + width, diff_function(grid[i], grid[i + width])});
    }
    return std::make_unique<Graph>(std::move(keys), std::move(edges));
  }

  std::unique_ptr<Graph> GetMinimumSpanningTree() const;

  const std::vector<NodeEdge>& GetEdges() const;

  std::unique_ptr<Graph> GetSubGraph(
      const std::unordered_set<size_t>& keys) const;

  NodeEdge GetMinDistance() const;

  NodeEdge GetMaxDistance() const;

  bool ContainsNode(size_t key) const;

  const std::unordered_set<size_t>& Keys() const { return keys_; }

 private:
  void SortEdges();

  std::unordered_set<size_t> keys_;
  std::vector<NodeEdge> edges_;
};

}  // namespace base

#endif  // SRC_BASE_GRAPH_H_