#pragma once

#include <map>
#include <unordered_set>

#include <spdlog/spdlog.h>

#include <core/utils/string_token.hpp>

namespace hs::utils {

// Structure keeps count of how many instances of given id are present.
// It can answer the following queries:
// 1. How many instances of given id were added
// 2. What are top N occuring instances
template <typename BaseTypes> class Comb {
public:
  using StringId = typename BaseTypes::StringId;
  Comb() = default;

  void Add(const StringId &elem) {
    if (BaseTypes::IsNullToken(elem)) {
      return;
    }
    auto prev_count = count_[elem]++;
    auto curr_count = prev_count + 1;
    comb_[prev_count].erase(elem);
    comb_[curr_count].insert(elem);
  }

  void Remove(const StringId &elem) {
    if (BaseTypes::IsNullToken(elem)) {
      return;
    }
    auto prev_count = count_[elem]--;
    auto curr_count = prev_count - 1;
    comb_[prev_count].erase(elem);
    comb_[curr_count].insert(elem);
  }

  int Count(const StringId &elem) const {
    auto fit = count_.find(elem);
    if (fit != count_.end()) {
      return fit->second;
    }
    return 0;
  }

  std::vector<std::pair<StringId, int>> TopK(int k) const {
    std::vector<std::pair<StringId, int>> result;
    if (k <= 0) {
      return result;
    }
    result.reserve(k);
    for (auto it = comb_.rbegin(); it != comb_.rend(); ++it) {
      if (it->first <= 0) {
        continue;
      }
      for (const auto &elem : it->second) {
        result.emplace_back(std::make_pair(elem, it->first));
        k--;

        if (k <= 0) {
          break;
        }
      }

      if (k <= 0) {
        break;
      }
    }

    return result;
  }

  void clear() { Clear(); }

  void Clear() {
    comb_.clear();
    count_.clear();
  }

  bool operator==(const Comb &other) const noexcept;
  bool operator!=(const Comb &other) const noexcept {
    return !(*this == other);
  }

private:
  std::map<int, std::unordered_set<StringId>> comb_;
  std::unordered_map<StringId, int> count_;
};

} // namespace hs::utils

#include "comb.inl"
