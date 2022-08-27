#ifndef SRC_SELECTIVE_SEARCH_SELECTIVE_SEARCH_H_
#define SRC_SELECTIVE_SEARCH_SELECTIVE_SEARCH_H_

#include <functional>
#include <list>
#include <memory>
#include <numeric>

#include "base/graph.h"

namespace selective_search {

struct Component {
  size_t component_id;
  size_t component_size;
  float internal_difference = std::numeric_limits<float>::quiet_NaN();
};

// http://vision.stanford.edu/teaching/cs231b_spring1415/papers/IJCV2004_FelzenszwalbHuttenlocher.pdf
// https://ivi.fnwi.uva.nl/isis/publications/2013/UijlingsIJCV2013/UijlingsIJCV2013.pdf
std::list<Component> SelectiveSearch(
    const base::Graph& g,
    std::function<float(const Component&)> threshold_function);

}  // namespace selective_search

#endif  // SRC_SELECTIVE_SEARCH_SELECTIVE_SEARCH_H_