#pragma once

#include <absl/container/flat_hash_map.h>
#include <render/atlas.pb.h>
#include <render/render.pb.h>
#include <ruleset/biome.pb.h>
#include <ruleset/jobs.pb.h>
#include <ruleset/effect.pb.h>
#include <ruleset/projects.pb.h>
#include <ruleset/region_improvements.pb.h>
#include <ruleset/resources.pb.h>
#include <ruleset/variables.pb.h>

#include <core/utils/error_message.hpp>
#include <core/utils/string_token.hpp>
#include <filesystem>

namespace hs::ruleset {

class RuleSetBase {
 public:
  using ErrorsCollection = utils::ErrorsCollection;
  void Clear();
  // Adds data to ruleset
  bool Load(const std::filesystem::path &path, ErrorsCollection &errors);

  auto &GetRegionImprovements() const { return improvements_; }

  auto &GetBiomes() const { return biomes_; }

  auto &GetResources() const { return resources_; }

  auto &GetRendering() const { return rendering_; }

  auto &GetJobs() const { return jobs_; }

  auto &GetProjects() const { return projects_; }

  auto &GetAllEffects() const { return effects_; }

  auto &GetVariableDefinitions() const { return variable_definitions_; }

 protected:
  proto::ruleset::RegionImprovements improvements_;
  proto::ruleset::Biomes biomes_;
  proto::ruleset::Resources resources_;
  proto::ruleset::Jobs jobs_;
  proto::ruleset::Projects projects_;
  proto::render::Rendering rendering_;
  proto::ruleset::Variables variable_definitions_;
  std::vector<proto::ruleset::effect::Effect> effects_;

  static inline std::filesystem::path improvements_file{
      "region_improvements.txt"};
  static inline std::filesystem::path biomes_file{"biomes.txt"};
  static inline std::filesystem::path resources_file{"resources.txt"};
  static inline std::filesystem::path rendering_file{"rendering.txt"};
  static inline std::filesystem::path jobs_file{"jobs.txt"};
  static inline std::filesystem::path projects_file{"projects.txt"};
  static inline std::filesystem::path static_effects_file{"static_effects.txt"};
  static inline std::filesystem::path variable_definitions_file{
      "variable_definitions.txt"};
};

}  // namespace hs::ruleset
