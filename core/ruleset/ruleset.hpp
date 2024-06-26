#pragma once

#include <string_view>
#include <filesystem>

#include <absl/container/flat_hash_map.h>

#include <core/utils/string_token.hpp>
#include <core/utils/error_message.hpp>

#include <ruleset/region_improvements.pb.h>
#include <ruleset/biome.pb.h>
#include <ruleset/jobs.pb.h>
#include <ruleset/projects.pb.h>
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

  auto& GetBiomes() const { return biomes_; }

  auto& GetResources() const { return resources_; }

  auto& GetRendering() const { return rendering_; }

  auto& GetJobs() const { return jobs_; }

  auto& GetProjects() const { return projects_; }

  const proto::ruleset::RegionImprovement* FindRegionImprovementByType(
    utils::StringTokenCRef improvement_type_id) const;

  const proto::ruleset::Job* FindJobByType(
    utils::StringTokenCRef job_type_id) const;

  const proto::ruleset::Project* FindProjectByType(
    utils::StringTokenCRef project_type_id) const;


private:
  proto::ruleset::RegionImprovements improvements_;
  proto::ruleset::Biomes biomes_;
  proto::ruleset::Resources resources_;
  proto::ruleset::Jobs jobs_;
  proto::ruleset::Projects projects_;
  proto::render::Rendering rendering_;

  absl::flat_hash_map<utils::StringToken, size_t> improvements_by_type_;
  absl::flat_hash_map<utils::StringToken, size_t> jobs_by_type_;
  absl::flat_hash_map<utils::StringToken, size_t> projects_by_type_;

  static inline std::filesystem::path improvements_file{"region_improvements.txt"};
  static inline std::filesystem::path biomes_file{"biomes.txt"};
  static inline std::filesystem::path resources_file{"resources.txt"};
  static inline std::filesystem::path rendering_file{"rendering.txt"};
  static inline std::filesystem::path jobs_file{"jobs.txt"};
  static inline std::filesystem::path projects_file{"projects.txt"};
};

}
