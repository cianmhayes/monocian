#ifndef SRC_BASE_MERGE_H_
#define SRC_BASE_MERGE_H_

#include <functional>
#include <list>

namespace base {

template <typename TValue>
std::list<TValue> Merge(std::list<TValue>&& candidates,
                        std::function<bool(const TValue&)> is_mergable,
                        std::function<TValue(std::list<TValue>&&)> merge,
                        size_t merge_limit) {
  std::list<TValue> result = {};
  std::list<TValue> mergable = {};
  while (!candidates.empty()) {
    if (mergable.size() < merge_limit && is_mergable(candidates.front())) {
      mergable.splice(mergable.begin(), candidates, candidates.begin());
    } else {
      result.splice(result.begin(), candidates, candidates.begin());
    }
  }
  if (mergable.size() > 1) {
    result.push_back(merge(std::move(mergable)));
  } else if (mergable.size() == 1) {
    result.splice(result.begin(), mergable, mergable.begin());
  }
  
  return result;
}

template <typename TValue>
std::list<TValue> TryMerge(std::list<TValue>&& candidates,
                        std::function<bool(const TValue&)> is_mergable,
                        std::function<std::list<TValue>(std::list<TValue>&&)> merge,
                        size_t merge_limit) {
  std::list<TValue> result = {};
  std::list<TValue> mergable = {};
  while (!candidates.empty()) {
    if (mergable.size() < merge_limit && is_mergable(candidates.front())) {
      mergable.splice(mergable.begin(), candidates, candidates.begin());
    } else {
      result.splice(result.begin(), candidates, candidates.begin());
    }
  }
  if (mergable.size() > 1) {
    std::list<TValue> maybe_merged = merge(std::move(mergable));
    result.splice(result.begin(), maybe_merged, maybe_merged.begin(), maybe_merged.end());
  } else if (mergable.size() == 1) {
    result.splice(result.begin(), mergable, mergable.begin());
  }
  
  return result;
}

}  // namespace base

#endif  // SRC_BASE_MERGE_H_