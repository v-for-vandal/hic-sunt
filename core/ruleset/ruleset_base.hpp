#pragma once

#include <google/protobuf/repeated_ptr_field.h>
#include <render/atlas.pb.h>
#include <render/render.pb.h>
#include <ruleset/biome.pb.h>
#include <ruleset/effect.pb.h>
#include <ruleset/group.pb.h>
#include <ruleset/improvements.pb.h>
#include <ruleset/jobs.pb.h>
#include <ruleset/projects.pb.h>
#include <ruleset/resources.pb.h>
#include <ruleset/variables.pb.h>

#include <core/types/scope_type.hpp>
#include <core/utils/error_message.hpp>
#include <core/utils/string_token.hpp>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace hs::ruleset {

class RuleSetBase {
 public:
  using ErrorsCollection = utils::ErrorsCollection;
  void Clear();
  bool Load(const std::vector<std::filesystem::path>& paths, ErrorsCollection& errors);

  auto& GetRegionImprovements() const { return improvements_; }
  auto& GetBiomes() const { return biomes_; }
  auto& GetResources() const { return resources_; }
  auto& GetRendering() const { return rendering_; }
  auto& GetJobs() const { return jobs_; }
  auto& GetProjects() const { return projects_; }
  auto& GetAllEffects() const { return effects_; }
  auto& GetVariableDefinitions() const { return variable_definitions_; }

 protected:
  static bool IsValidIdentifier(std::string_view identifier);
  static bool ValidateIdentifier(std::string_view identifier, std::string_view context,
                                 ErrorsCollection& errors);
  bool ValidateGroups(ErrorsCollection& errors) const;

 private:
  bool ValidateGroupReferences(std::string_view owner_kind, const std::string& owner_id,
                               const google::protobuf::RepeatedPtrField<std::string>& group_ids,
                               types::ScopeType expected_type, ErrorsCollection& errors) const;
  bool ValidateGroupGraph(const google::protobuf::RepeatedPtrField<proto::ruleset::Group>& groups,
                          std::string_view kind, types::ScopeType expected_type,
                          ErrorsCollection& errors) const;

 protected:
  proto::ruleset::Improvements improvements_;
  proto::ruleset::Biomes biomes_;
  proto::ruleset::Resources resources_;
  proto::ruleset::Jobs jobs_;
  proto::ruleset::Projects projects_;
  proto::render::Rendering rendering_;
  proto::ruleset::Variables variable_definitions_;
  std::vector<proto::ruleset::effect::Effect> effects_;

  static inline std::filesystem::path improvements_dir{"improvements"};
  static inline std::filesystem::path biomes_dir{"biomes"};
  static inline std::filesystem::path resources_dir{"resources"};
  static inline std::filesystem::path rendering_dir{"rendering"};
  static inline std::filesystem::path jobs_dir{"jobs"};
  static inline std::filesystem::path projects_dir{"projects"};
  static inline std::filesystem::path effects_dir{"effects"};
  static inline std::filesystem::path variable_definitions_dir{"variables"};
};

}  // namespace hs::ruleset
