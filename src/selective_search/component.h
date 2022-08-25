#ifndef SRC_SELECTIVE_SEARCH_COMPONENT_H_
#define SRC_SELECTIVE_SEARCH_COMPONENT_H_

#include <numeric>
#include <unordered_set>
#include "base/graph.h"

namespace selective_search {

class Component {
 public:
  Component(size_t id, const base::Graph* g, std::unordered_set<size_t>&& elements);
  Component(const Component& lhs, const Component& rhs);
  ~Component();
  Component(const Component&) = delete;
  Component& operator=(const Component&) = delete;

  size_t Id() const;
  bool Contains(size_t element) const;
  float InternalDifference();
  size_t Size() const;

 private:
  size_t id_;
  const base::Graph* g_;
  std::unordered_set<size_t> elements_;
  float internal_difference_ = std::numeric_limits<float>::quiet_NaN();
};

}  // namespace selective_search

#endif  // SRC_SELECTIVE_SEARCH_COMPONENT_H_