#include "base/graph.h"

#include <algorithm>
#include <iterator>
#include <list>
#include <memory>
#include <unordered_set>

#include "base/merge.h"

namespace base {

Graph::Graph(std::unordered_set<size_t>&& keys, std::vector<NodeEdge>&& edges)
    : keys_(keys), edges_(edges) {
  SortEdges();
};
Graph::Graph(const Graph& lhs, const Graph& rhs, NodeEdge edge) {
  keys_ = {};
  for (const auto& e : lhs.keys_)
    keys_.insert(e);
  for (const auto& e : rhs.keys_)
    keys_.insert(e);
  edges_ = {};
  std::copy(lhs.edges_.begin(), lhs.edges_.end(), std::back_inserter(edges_));
  std::copy(rhs.edges_.begin(), rhs.edges_.end(), std::back_inserter(edges_));
  edges_.push_back(edge);
  SortEdges();
}

Graph::~Graph() = default;

void Graph::SortEdges() {
  std::sort(edges_.begin(), edges_.end(),
            [](const NodeEdge& lhs, const NodeEdge& rhs) {
              return lhs.weight < rhs.weight;
            });
}

bool Graph::ContainsNode(size_t key) const {
  return keys_.count(key) > 0;
}

std::unique_ptr<Graph> Graph::GetMinimumSpanningTree() const {
  // https://en.wikipedia.org/wiki/Kruskal%27s_algorithm
  // create a graph for each distinct key
  // for each edge, in ascending order of weight:
  //   find the graph or graphs that it touches
  //   if it would create a cycle on a graph, discard it
  //   if it would link two graphs, combine the graphs and add the edge
  // stop if there's a single graph left
  std::list<std::unique_ptr<Graph>> graphs = {};
  for (size_t key : keys_) {
    graphs.push_front(std::make_unique<Graph>(std::unordered_set<size_t>({key}),
                                              std::vector<NodeEdge>()));
  }
  for (const auto& edge : edges_) {
    graphs = Merge<std::unique_ptr<base::Graph>>(
        std::move(graphs),
        [&edge](const std::unique_ptr<Graph>& g) {
          return g->ContainsNode(edge.first) || g->ContainsNode(edge.second);
        },
        [&edge](std::list<std::unique_ptr<base::Graph>>&& mergable) {
          base::Graph* g1 = mergable.front().get();
          base::Graph* g2 = mergable.back().get();
          return std::make_unique<base::Graph>(*g1, *g2, edge);
        },
        2);
    if (graphs.size() == 1) {
      return std::move(graphs.front());
    }
  }
  return nullptr;
}

NodeEdge Graph::GetMinDistance() const {
  return edges_.front();
}

NodeEdge Graph::GetMaxDistance() const {
  return edges_.back();
}

const std::vector<NodeEdge>& Graph::GetEdges() const {
  return edges_;
}

std::unique_ptr<Graph> Graph::GetSubGraph(
    const std::unordered_set<size_t>& keys) const {
  std::unordered_set<size_t> keys_copy = {};
  std::vector<NodeEdge> edges = {};
  for (const auto& e : edges_) {
    if (keys.count(e.first) > 0 && keys.count(e.second) > 0) {
      keys_copy.insert(e.first);
      keys_copy.insert(e.second);
      edges.push_back(e);
    }
  }
  return std::make_unique<Graph>(std::move(keys_copy), std::move(edges));
}

}  // namespace base