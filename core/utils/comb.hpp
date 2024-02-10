#pragma once

#include <map>
#include <unordered_set>

#include <core/utils/string_token.hpp>

namespace hs::utils {

class Comb {
public:
  Comb() = default;

  void Add(StringTokenCRef elem) {
    auto prev_count = count_[elem]++;
    auto curr_count = prev_count+1;
    comb_[prev_count].erase(elem);
    comb_[curr_count].insert(elem);
  }

  void Remove(StringTokenCRef elem) {
    auto prev_count = count_[elem]--;
    auto curr_count = prev_count-1;
    comb_[prev_count].erase(elem);
    comb_[curr_count].insert(elem);
  }

  int Count(StringTokenCRef elem) const {
    auto fit = count_.find(elem);
    if( fit != count_.end()) {
      return fit->second;
    }
    return 0;
  }

  std::vector<std::pair<StringToken,int>> TopK(int k) const {
    std::vector<std::pair<StringToken,int>> result;
    if(k <= 0) {
      return result;
    }
    result.reserve(k);
    for(auto it = comb_.rbegin(); it != comb_.rend(); ++it) {
      if(it->first <= 0) {
        continue;
      }
      for(const auto& elem : it->second) {
        result.emplace_back(std::make_pair(elem, it->first));
        k--;

        if(k <= 0) {
          break;
        }
      }

      if (k <= 0) {
        break;
      }
    }

    return result;
  }




private:
  std::map<int, std::unordered_set<StringToken>> comb_;
  std::unordered_map<StringToken, int> count_;

};

}
