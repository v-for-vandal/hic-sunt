#pragma once

#include <string_view>
#include <filesystem>

#include <absl/container/flat_hash_map.h>

#include <core/utils/string_token.hpp>
#include <core/utils/error_message.hpp>

#include <ruleset/region_improvements.pb.h>
#include <ruleset/terrain.pb.h>
#include <ruleset/resources.pb.h>
#include <render/render.pb.h>
#include <render/atlas.pb.h>

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

  auto& GetRendering() const { return rendering_; }

  const proto::ruleset::RegionImprovement* FindRegionImprovementByType(
    utils::StringTokenCRef improvement_type_id) const;


private:
  proto::ruleset::RegionImprovements improvements_;
  proto::ruleset::Terrain terrain_;
  proto::ruleset::Resources resources_;
  proto::render::Rendering rendering_;

  absl::flat_hash_map<utils::StringToken, size_t> improvements_by_type_;

  static inline std::filesystem::path improvements_file{"region_improvements.txt"};
  static inline std::filesystem::path terrain_file{"terrain.txt"};
  static inline std::filesystem::path resources_file{"resources.txt"};
  static inline std::filesystem::path rendering_file{"rendering.txt"};
};

}
