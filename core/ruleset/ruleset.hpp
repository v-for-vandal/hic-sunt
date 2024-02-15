#pragma once

#include <string_view>
#include <filesystem>

#include <core/utils/error_message.hpp>

#include <ruleset/region_improvements.pb.h>
#include <ruleset/terrain.pb.h>
#include <ruleset/resources.pb.h>

namespace hs::ruleset {

class RuleSet {
public:
  using ErrorsCollection = utils::ErrorsCollection;
  void Clear();
  // Adds data to ruleset
  bool Load(const std::filesystem::path& path, ErrorsCollection& errors);

  auto& GetRegionImprovements() const { return improvements_; }

  auto& GetTerrain() const { return terrain_; }

  auto& GetResources() const { return resources_; }


private:
  proto::ruleset::RegionImprovements improvements_;
  proto::ruleset::Terrain terrain_;
  proto::ruleset::Resources resources_;

  static inline std::filesystem::path improvements_file{"region_improvements.txt"};
  static inline std::filesystem::path terrain_file{"terrain.txt"};
  static inline std::filesystem::path resources_file{"resources.txt"};
};

}
