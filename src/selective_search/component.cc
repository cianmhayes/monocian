#include "selective_search/component.h"

#include <algorithm>
#include <cmath>
#include <iterator>

namespace selective_search {

Component::Component(size_t id,
                     const base::Graph* g,
                     std::unordered_set<size_t>&& elements)
    : id_(id), g_(g), elements_(elements) {
    }

Component::Component(const Component& lhs, const Component& rhs) {
  elements_ = {};
  for (const auto& e : lhs.elements_)
    elements_.insert(e);
  for (const auto& e : rhs.elements_)
    elements_.insert(e);
  g_ = lhs.g_;
  id_ = lhs.id_;
}

Component::~Component() = default;

bool Component::Contains(size_t element) const {
  return elements_.count(element) > 0;
}

float Component::InternalDifference() {
  if (std::isnan(internal_difference_)) {
    if (elements_.size() <= 1) {
      internal_difference_ = 0.0f;
    } else {
      auto sub_graph = g_->GetSubGraph(elements_);
      auto mst = sub_graph->GetMinimumSpanningTree();
      base::NodeEdge largest_existing_distance = mst->GetMaxDistance();
      internal_difference_ = largest_existing_distance.weight;
    }
  }
  return internal_difference_;
}

size_t Component::Size() const {
  return elements_.size();
}

size_t Component::Id() const {
  return id_;
}

}  // namespace selective_search