#include "selective_search/selective_search.h"

#include <algorithm>
#include <list>
#include <memory>

#include "base/merge.h"

namespace selective_search {

std::list<std::unique_ptr<Component>> SelectiveSearch(
    const base::Graph& g,
    std::function<float(const Component&)> threshold_function) {

  std::list<std::unique_ptr<Component>> segmentation = {};
  for (const auto& k : g.Keys()) {
    std::unordered_set<size_t> elements = {k};
    segmentation.push_back(
        std::make_unique<Component>(k, &g, std::move(elements)));
  }
  const std::vector<base::NodeEdge>& edges = g.GetEdges();
  for (const auto& e : edges) {
    segmentation = base::TryMerge<std::unique_ptr<Component>>(
        std::move(segmentation),
        [&e](const std::unique_ptr<Component>& c) {
          return c->Contains(e.first) || c->Contains(e.second);
        },
        [&e, &threshold_function](std::list<std::unique_ptr<Component>>&& mergable) {
          Component* c1 = mergable.front().get();
          Component* c2 = mergable.back().get();
          float c1_weight = c1->InternalDifference();
          c1_weight += threshold_function(*c1);
          float c2_weight = c2->InternalDifference();
          c2_weight += threshold_function(*c2);
          float min_weight = c1_weight > c2_weight ? c2_weight : c1_weight;
          if (e.weight <= min_weight) {
            std::list<std::unique_ptr<Component>> merged = {};
            merged.push_back(std::make_unique<Component>(*c1, *c2));
            return merged;
          } else {
            return mergable;
          }
        },
        2);
  }
  return segmentation;
}

}  // namespace selective_search
