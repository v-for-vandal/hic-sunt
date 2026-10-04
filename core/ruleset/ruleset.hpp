#pragma once

#include <absl/container/flat_hash_map.h>
#include <google/protobuf/repeated_ptr_field.h>
#include <ruleset/group.pb.h>
#include <ruleset/ruleset.pb.h>

#include <core/ruleset/effect.hpp>
#include <core/ruleset/ruleset_base.hpp>
#include <core/ruleset/variable_definition.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/error_message.hpp>
#include <core/utils/serialize.hpp>
#include <filesystem>
#include <optional>
#include <string_view>

namespace hs::ruleset {

template <typename BaseTypes>
class RuleSet;

template <typename BaseTypes>
void SerializeTo(const RuleSet<BaseTypes>& source, proto::ruleset::RuleSet& target);

template <typename BaseTypes>
RuleSet<BaseTypes> ParseFrom(const proto::ruleset::RuleSet& source,
                             serialize::To<RuleSet<BaseTypes>>);

template <typename BaseTypes = StdBaseTypes>
class RuleSet : public RuleSetBase {
 public:
  using ErrorsCollection = utils::ErrorsCollection;
  using StringId = typename BaseTypes::StringId;
  void Clear();
  // Adds data to ruleset
  bool Load(const std::vector<std::filesystem::path>& paths, ErrorsCollection& errors);

  bool DumpVariablesCsv(const std::filesystem::path& file_path, ErrorsCollection& errors) const;

  const proto::ruleset::Improvement* FindRegionImprovementByType(
      const StringId& improvement_type_id) const;

  const proto::ruleset::Resource* FindResourceByType(const StringId& resource_type_id) const;

  const proto::ruleset::Job* FindJobByType(const StringId& job_type_id) const;

  struct GroupDefinition {
    const proto::ruleset::Group* group;
    types::ScopeType scope_type;
  };
  std::optional<GroupDefinition> FindGroupById(const StringId& group_id) const;

  const proto::ruleset::Project* FindProjectByType(const StringId& project_type_id) const;

  const VariableDefinitionsPtr<BaseTypes>& GetVariableDefinitions() const {
    return parsed_variable_definitions_;
  }

  const auto& GetAllEffectDefinitions() const noexcept { return effect_definitions_; }

  // Functions that generates fixed scope id

  /* \brief Returns scope id for scope SCOPE_TYPE_IMPROVEMENT_CLASS for improvement id and civ id
   *
   */
  static StringId ImprovementClassScopeId(StringId civ_id, StringId improvement_class);
  static StringId JobClassScopeId(StringId civ_id, StringId job_class);
  static StringId GroupScopeId(StringId civ_id, StringId group_id);

 private:
  friend void SerializeTo<BaseTypes>(const RuleSet& source, proto::ruleset::RuleSet& target);
  friend RuleSet ParseFrom<BaseTypes>(const proto::ruleset::RuleSet& source,
                                      serialize::To<RuleSet>);

  static bool IsValidIdentifier(std::string_view identifier);
  static bool ValidateIdentifier(std::string_view identifier, std::string_view context,
                                 ErrorsCollection& errors);

  bool LoadImprovements(ErrorsCollection& errors);
  bool LoadResources(ErrorsCollection& errors);
  bool LoadJobs(ErrorsCollection& errors);
  bool AddGroup(const proto::ruleset::Group& group, types::ScopeType scope_type, size_t index,
                ErrorsCollection& errors);
  bool ValidateGroups(ErrorsCollection& errors) const;
  bool ValidateGroupReferences(std::string_view owner_kind, const std::string& owner_id,
                               const google::protobuf::RepeatedPtrField<std::string>& group_ids,
                               types::ScopeType expected_type, ErrorsCollection& errors) const;
  bool ValidateGroupGraph(const google::protobuf::RepeatedPtrField<proto::ruleset::Group>& groups,
                          std::string_view kind, types::ScopeType expected_type,
                          ErrorsCollection& errors) const;
  bool LoadProjects([[maybe_unused]] ErrorsCollection& errors);
  bool LoadEffects(ErrorsCollection& errors);
  bool LoadVariableDefinitions(ErrorsCollection& errors);

  absl::flat_hash_map<StringId, size_t> improvements_by_type_;
  absl::flat_hash_map<StringId, size_t> resources_by_id_;
  absl::flat_hash_map<StringId, size_t> jobs_by_type_;
  struct GroupIndexEntry {
    types::ScopeType scope_type;
    size_t index;
  };
  absl::flat_hash_map<StringId, GroupIndexEntry> groups_by_id_;
  absl::flat_hash_map<StringId, size_t> projects_by_type_;
  std::vector<StringId> resource_ids_;
  std::vector<StringId> job_ids_;

  VariableDefinitionsPtr<BaseTypes> parsed_variable_definitions_;
  std::vector<ConstEffectDefinitionPtr<BaseTypes>> effect_definitions_;
};

}  // namespace hs::ruleset

#include "ruleset.inl"
