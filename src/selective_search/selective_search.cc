#include "selective_search/selective_search.h"

#include <algorithm>
#include <list>
#include <map>
#include <memory>

#include "base/merge.h"

namespace selective_search {

std::list<Component> SelectiveSearch(
    const base::Graph& g,
    std::function<float(const Component&)> threshold_function) {
  // segment -> pixel key
  std::multimap<size_t, size_t> seg_to_value_map = {};
  std::map<size_t, size_t> value_to_seg_map = {};
  std::list<Component> segmentation = {};
  for (const auto& k : g.Keys()) {
    std::unordered_set<size_t> elements = {k};
    seg_to_value_map.emplace(k, k);
    value_to_seg_map.emplace(k, k);
    segmentation.push_back({k, 0, 0.0f});
  }
  const std::vector<base::NodeEdge>& edges = g.GetEdges();
  std::multimap<size_t, size_t>* seg_to_value_map_ptr = &seg_to_value_map;
  std::map<size_t, size_t>* value_to_seg_map_ptr = &value_to_seg_map;
  for (const auto& e : edges) {
    base::TryMerge<Component>(
        segmentation,
        [&](const Component& c) {
          return (*value_to_seg_map_ptr)[e.first] == c.component_id ||
                 (*value_to_seg_map_ptr)[e.second] == c.component_id;
        },
        [&](std::list<Component>&& mergable) {
          const Component& c1 = mergable.front();
          const Component& c2 = mergable.back();
          float c1_weight = c1.internal_difference;
          c1_weight += threshold_function(c1);
          float c2_weight = c2.internal_difference;
          c2_weight += threshold_function(c2);
          float min_weight = c1_weight > c2_weight ? c2_weight : c1_weight;
          if (e.weight <= min_weight) {
            std::list<Component> merged = {};
            float new_difference =
                c1.internal_difference > c2.internal_difference
                    ? c1.internal_difference
                    : c2.internal_difference;
            new_difference =
                new_difference > e.weight ? new_difference : e.weight;
            size_t new_id = c1.component_id;
            size_t replaced_id = c2.component_id;
            if (c1.component_id > c2.component_id) {
              new_id = c2.component_id;
              replaced_id = c1.component_id;
            }
            auto range_to_update =
                seg_to_value_map_ptr->equal_range(replaced_id);
            for (auto i = range_to_update.first; i != range_to_update.second;
                 ++i) {
              (*value_to_seg_map_ptr)[i->second] = new_id;
            }
            auto reassigned = seg_to_value_map_ptr->extract(replaced_id);
            reassigned.key() = new_id;
            seg_to_value_map_ptr->insert(std::move(reassigned));
            merged.push_back(
                {new_id, seg_to_value_map_ptr->count(new_id), new_difference});
            return merged;
          } else {
            return mergable;
          }
        });
  }
  return segmentation;
}

}  // namespace selective_search
