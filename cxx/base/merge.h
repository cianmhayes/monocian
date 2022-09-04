#ifndef BASE_MERGE_H_
#define BASE_MERGE_H_

#include <functional>
#include <list>

namespace base {

template <typename TValue>
void Merge(std::list<TValue>& candidates,
           std::function<bool(const TValue&)> is_mergable,
           std::function<TValue(std::list<TValue>&&)> merge) {
  std::list<TValue> mergable = {};
  for (int i = 0; i < 2; i++) {
    for (auto it = candidates.begin(); it != candidates.end(); it++) {
      if (is_mergable(*it)) {
        mergable.splice(mergable.begin(), candidates, it);
        break;
      }
    }
  }
  if (mergable.size() > 1) {
    candidates.push_back(merge(std::move(mergable)));
  } else if (mergable.size() == 1) {
    candidates.splice(candidates.begin(), mergable, mergable.begin());
  }
}

template <typename TValue>
void TryMerge(std::list<TValue>& candidates,
              std::function<bool(const TValue&)> is_mergable,
              std::function<std::list<TValue>(std::list<TValue>&&)> merge) {
  std::list<TValue> mergable = {};
  for (int i = 0; i < 2; i++) {
    for (auto it = candidates.begin(); it != candidates.end(); it++) {
      if (is_mergable(*it)) {
        mergable.splice(mergable.begin(), candidates, it);
        break;
      }
    }
  }
  if (mergable.size() > 1) {
    std::list<TValue> maybe_merged = merge(std::move(mergable));
    candidates.splice(candidates.begin(), maybe_merged, maybe_merged.begin(),
                      maybe_merged.end());
  } else if (mergable.size() == 1) {
    candidates.splice(candidates.begin(), mergable, mergable.begin());
  }
}

}  // namespace base

#endif  // BASE_MERGE_H_