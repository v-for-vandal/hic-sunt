#pragma once

#include <string_view>
#include <filesystem>

#include <core/utils/error_message.hpp>

#include <region/improvements.pb.h>

namespace hs::ruleset {

class RuleSet {
public:
  using ErrorsCollection = utils::ErrorsCollection;
  void Clear();
  // Adds data to ruleset
  bool Load(const std::filesystem::path& path, ErrorsCollection& errors);

  auto& GetRegionImprovements() const { return improvements_; }


private:
  proto::region::Improvements improvements_;

  static inline std::filesystem::path improvements_file{"improvements.txt"};
};

}
