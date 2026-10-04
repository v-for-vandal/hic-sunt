#pragma once

#include <absl/container/flat_hash_set.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>

#include "ruleset.hpp"
#include "spdlog/spdlog.h"
namespace hs::ruleset {

namespace {

inline void AddWarning(utils::ErrorsCollection& errors, const std::string& message) {
  spdlog::warn(message);
  errors.AddError({message});
}

inline void AddError(utils::ErrorsCollection& errors, const std::string& message) {
  spdlog::error(message);
  errors.AddError({message});
}

inline std::string CsvEscape(std::string_view value) {
  const bool must_quote = value.find_first_of(",\n\r\"") != std::string_view::npos;
  if (!must_quote) {
    return std::string{value};
  }

  std::string result;
  result.reserve(value.size() + 2);
  result.push_back('"');
  for (const char c : value) {
    if (c == '"') {
      result.push_back('"');
    }
    result.push_back(c);
  }
  result.push_back('"');
  return result;
}

template <typename BaseTypes>
std::string BuildDumpedParameterizedVariableId(
    const typename BaseTypes::StringId& pattern,
    const std::vector<std::pair<ParameterDomainKind, typename BaseTypes::StringId>>&
        parameter_values) {
  const std::string pattern_string = BaseTypes::ToProtoString(pattern);
  std::string result;
  result.reserve(pattern_string.size());

  size_t parameter_index = 0;
  for (size_t pos = 0; pos < pattern_string.size();) {
    if (pattern_string[pos] != '{') {
      result.push_back(pattern_string[pos]);
      ++pos;
      continue;
    }

    const auto close_pos = pattern_string.find('}', pos);
    if (close_pos == std::string::npos || parameter_index >= parameter_values.size()) {
      result.append(pattern_string.substr(pos));
      break;
    }

    const auto& [kind, value] = parameter_values[parameter_index++];
    if (kind == ParameterDomainKind::kOpen) {
      result.append("{}");
    } else {
      result.push_back('@');
      result.append(BaseTypes::ToProtoString(value));
    }
    pos = close_pos + 1;
  }

  return result;
}

template <typename BaseTypes, typename ConcreteDefinition, typename CollectFn>
void ForEachDumpedParameterizedVariable(
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition,
    CollectFn&& collect_fn) {
  using StringId = typename BaseTypes::StringId;
  std::vector<std::pair<ParameterDomainKind, StringId>> parameter_values;
  parameter_values.reserve(definition.parameters.size());

  auto visit = [&](this auto&& self, size_t parameter_index) -> void {
    if (parameter_index == definition.parameters.size()) {
      collect_fn(
          BuildDumpedParameterizedVariableId<BaseTypes>(definition.pattern, parameter_values));
      return;
    }

    const auto& parameter = definition.parameters[parameter_index];
    if (parameter.kind == ParameterDomainKind::kOpen) {
      parameter_values.emplace_back(ParameterDomainKind::kOpen, StringId{});
      self(parameter_index + 1);
      parameter_values.pop_back();
      return;
    }

    if (parameter.fixed_values == nullptr) {
      return;
    }
    for (const auto& value : parameter.fixed_values->ordered_values) {
      parameter_values.emplace_back(ParameterDomainKind::kFixed, value);
      self(parameter_index + 1);
      parameter_values.pop_back();
    }
  };

  visit(0);
}

}  // namespace

template <typename BaseTypes>
bool RuleSet<BaseTypes>::Load(const std::vector<std::filesystem::path>& paths,
                              ErrorsCollection& errors) {
  Clear();

  if (!RuleSetBase::Load(paths, errors)) {
    return false;
  }

  bool success = true;
  success &= LoadImprovements(errors);
  success &= LoadResources(errors);
  success &= LoadJobs(errors);
  success &= ValidateGroups(errors);
  success &= LoadProjects(errors);
  success &= LoadVariableDefinitions(errors);
  if (!success) {
    return false;
  }

  return LoadEffects(errors);
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::DumpVariablesCsv(const std::filesystem::path& file_path,
                                          ErrorsCollection& errors) const {
  struct Row {
    std::string name;
    std::string type;
  };

  std::vector<Row> rows;

  for (const auto& [id, _] : parsed_variable_definitions_->GetNumericDefinitions()) {
    rows.push_back({.name = BaseTypes::ToProtoString(id), .type = "numeric"});
  }
  for (const auto& [id, _] : parsed_variable_definitions_->GetStringDefinitions()) {
    rows.push_back({.name = BaseTypes::ToProtoString(id), .type = "string"});
  }
  for (const auto& [_, definition] :
       parsed_variable_definitions_->GetParameterizedNumericDefinitions()) {
    ForEachDumpedParameterizedVariable<BaseTypes>(definition, [&rows](const auto& variable_id) {
      rows.push_back({.name = BaseTypes::ToProtoString(variable_id), .type = "numeric"});
    });
  }
  for (const auto& [_, definition] :
       parsed_variable_definitions_->GetParameterizedStringDefinitions()) {
    ForEachDumpedParameterizedVariable<BaseTypes>(definition, [&rows](const auto& variable_id) {
      rows.push_back({.name = BaseTypes::ToProtoString(variable_id), .type = "string"});
    });
  }

  std::ranges::sort(rows, {}, [](const Row& row) { return std::tie(row.name, row.type); });

  std::ofstream out(file_path);
  if (!out.is_open()) {
    AddError(errors, fmt::format("Can not open variables CSV file {}", file_path.string()));
    return false;
  }

  out << "name,type\n";
  for (const auto& row : rows) {
    out << CsvEscape(row.name) << ',' << CsvEscape(row.type) << '\n';
  }
  return true;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::IsValidIdentifier(std::string_view identifier) {
  return !identifier.empty() && std::ranges::all_of(identifier, [](char character) {
    return (character >= 'a' && character <= 'z') || (character >= '0' && character <= '9') ||
           character == '.';
  });
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::ValidateIdentifier(std::string_view identifier, std::string_view context,
                                            ErrorsCollection& errors) {
  if (IsValidIdentifier(identifier)) {
    return true;
  }

  AddError(errors,
           fmt::format("Identifier '{}' for {} is invalid; identifiers must contain only lowercase "
                       "ASCII letters, digits, and '.'",
                       identifier, context));
  return false;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadImprovements(ErrorsCollection& errors) {
  bool success = true;
  for (int idx = 0; idx < improvements_.improvements_size(); ++idx) {
    const auto& improvement = improvements_.improvements(idx);
    if (ValidateIdentifier(improvement.id(), "improvement", errors)) {
      improvements_by_type_.try_emplace(BaseTypes::StringIdFromStdString(improvement.id()), idx);
    } else {
      success = false;
    }

    for (const auto& [job_id, _] : improvement.jobs()) {
      success &= ValidateIdentifier(
          job_id, fmt::format("job reference in improvement '{}'", improvement.id()), errors);
    }
  }

  for (int idx = 0; idx < improvements_.improvement_groups_size(); ++idx) {
    success &=
        AddGroup(improvements_.improvement_groups(idx),
                 types::ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP, static_cast<size_t>(idx), errors);
  }

  return success;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadResources(ErrorsCollection& errors) {
  bool success = true;
  resource_ids_.reserve(resources_.resources_size());
  for (int idx = 0; idx < resources_.resources_size(); ++idx) {
    const auto& resource = resources_.resources(idx);
    if (!ValidateIdentifier(resource.id(), "resource", errors)) {
      success = false;
      continue;
    }

    const auto resource_id = BaseTypes::StringIdFromStdString(resource.id());
    resources_by_id_.try_emplace(resource_id, idx);
    resource_ids_.push_back(resource_id);
  }

  return success;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadJobs(ErrorsCollection& errors) {
  bool success = true;
  job_ids_.reserve(jobs_.jobs_size());
  for (int idx = 0; idx < jobs_.jobs_size(); ++idx) {
    const auto& job = jobs_.jobs(idx);
    const bool valid_job_id = ValidateIdentifier(job.id(), "job", errors);
    if (valid_job_id) {
      const auto job_id = BaseTypes::StringIdFromStdString(job.id());
      jobs_by_type_.try_emplace(job_id, idx);
      job_ids_.push_back(job_id);
      spdlog::debug("Working with job {}", job_id);
    } else {
      success = false;
    }

    for (const auto& [resource_id, _] : job.input()) {
      success &= ValidateIdentifier(
          resource_id, fmt::format("input resource reference in job '{}'", job.id()), errors);
    }
    for (const auto& [resource_id, _] : job.output()) {
      success &= ValidateIdentifier(
          resource_id, fmt::format("output resource reference in job '{}'", job.id()), errors);
    }

    if (!valid_job_id) {
      continue;
    }

    NumericVariableDefinition<BaseTypes> count_definition;
    count_definition.allowed_scopes.reset();
    count_definition.allowed_scopes |=
        types::ToScopeTypeFilter(types::ScopeTypeSet::SCOPE_TYPE_SET_JOBS);
    count_definition.allowed_scopes.set(types::ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP);
    count_definition.minimum = 0;

    const auto count_variable_id = fmt::format("job/{}/count", job.id());
    const auto add_result = parsed_variable_definitions_->AddNumericDefinition(
        BaseTypes::StringIdFromStdString(count_variable_id), count_definition);
    if (!add_result) {
      AddError(errors,
               fmt::format("Variable {} has conflicting type definition", count_variable_id));
      success = false;
      continue;
    }

    spdlog::debug("Added variable {}", count_variable_id);
    spdlog::debug("Is this variable numeric? {}",
                  GetVariableDefinitions()->IsNumericVariable(
                      BaseTypes::StringIdFromStdString(count_variable_id)));
  }

  for (int idx = 0; idx < jobs_.job_groups_size(); ++idx) {
    success &= AddGroup(jobs_.job_groups(idx), types::ScopeType::SCOPE_TYPE_JOB_GROUP,
                        static_cast<size_t>(idx), errors);
  }

  return success;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::AddGroup(const proto::ruleset::Group& group, types::ScopeType scope_type,
                                  size_t index, ErrorsCollection& errors) {
  if (!ValidateIdentifier(group.id(), "group", errors)) {
    return false;
  }

  const auto group_id = BaseTypes::StringIdFromStdString(group.id());
  const auto [existing, inserted] =
      groups_by_id_.try_emplace(group_id, GroupIndexEntry{scope_type, index});
  if (inserted) {
    return true;
  }

  AddError(errors,
           fmt::format("Group id '{}' is not unique in the merged ruleset: it is used by {} and {}",
                       group.id(), existing->second.scope_type, scope_type));
  return false;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::ValidateGroupReferences(
    std::string_view owner_kind, const std::string& owner_id,
    const google::protobuf::RepeatedPtrField<std::string>& group_ids,
    types::ScopeType expected_type, ErrorsCollection& errors) const {
  bool success = true;
  absl::flat_hash_set<std::string> unique_ids;
  for (const auto& group_id : group_ids) {
    if (!unique_ids.insert(group_id).second) {
      AddError(errors, fmt::format("{} '{}' contains duplicate group reference '{}'", owner_kind,
                                   owner_id, group_id));
      success = false;
    }

    if (!ValidateIdentifier(
            group_id, fmt::format("group reference in {} '{}'", owner_kind, owner_id), errors)) {
      success = false;
      continue;
    }

    const auto found = FindGroupById(BaseTypes::StringIdFromStdString(group_id));
    if (!found) {
      AddError(errors, fmt::format("{} '{}' references unknown group '{}'", owner_kind, owner_id,
                                   group_id));
      success = false;
      continue;
    }
    if (found->scope_type != expected_type) {
      AddError(errors,
               fmt::format("{} '{}' references group '{}' of type {}, expected {}", owner_kind,
                           owner_id, group_id, found->scope_type, expected_type));
      success = false;
    }
  }
  return success;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::ValidateGroupGraph(
    const google::protobuf::RepeatedPtrField<proto::ruleset::Group>& groups, std::string_view kind,
    types::ScopeType expected_type, ErrorsCollection& errors) const {
  bool success = true;
  absl::flat_hash_map<std::string, const proto::ruleset::Group*> by_id;
  by_id.reserve(static_cast<size_t>(groups.size()));
  for (const auto& group : groups) {
    if (IsValidIdentifier(group.id())) {
      by_id.try_emplace(group.id(), &group);
    }
    success &= ValidateGroupReferences(kind, group.id(), group.groups(), expected_type, errors);
  }

  enum class VisitState { kUnvisited, kVisiting, kVisited };
  absl::flat_hash_map<std::string, VisitState> states;
  std::vector<std::string> path;

  std::function<void(const proto::ruleset::Group&)> visit = [&](const auto& group) {
    auto& state = states[group.id()];
    if (state == VisitState::kVisited) {
      return;
    }
    if (state == VisitState::kVisiting) {
      const auto cycle_begin = std::ranges::find(path, group.id());
      std::string cycle;
      for (auto it = cycle_begin; it != path.end(); ++it) {
        if (!cycle.empty()) {
          cycle += " -> ";
        }
        cycle += *it;
      }
      if (!cycle.empty()) {
        cycle += " -> ";
      }
      cycle += group.id();
      AddError(errors, fmt::format("Cycle detected in {} graph: {}", kind, cycle));
      success = false;
      return;
    }

    state = VisitState::kVisiting;
    path.push_back(group.id());
    for (const auto& inherited_group_id : group.groups()) {
      const auto inherited = by_id.find(inherited_group_id);
      if (inherited != by_id.end()) {
        visit(*inherited->second);
      }
    }
    path.pop_back();
    state = VisitState::kVisited;
  };

  for (const auto& group : groups) {
    if (IsValidIdentifier(group.id())) {
      visit(group);
    }
  }

  return success;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::ValidateGroups(ErrorsCollection& errors) const {
  bool success = true;
  for (const auto& improvement : improvements_.improvements()) {
    success &= ValidateGroupReferences("Improvement", improvement.id(), improvement.groups(),
                                       types::ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP, errors);
  }
  for (const auto& job : jobs_.jobs()) {
    success &= ValidateGroupReferences("Job", job.id(), job.groups(),
                                       types::ScopeType::SCOPE_TYPE_JOB_GROUP, errors);
  }

  success &= ValidateGroupGraph(improvements_.improvement_groups(), "improvement group",
                                types::ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP, errors);
  success &= ValidateGroupGraph(jobs_.job_groups(), "job group",
                                types::ScopeType::SCOPE_TYPE_JOB_GROUP, errors);
  return success;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadProjects([[maybe_unused]] ErrorsCollection& errors) {
  for (int idx = 0; idx < projects_.projects_size(); ++idx) {
    const auto& project = projects_.projects(idx);
    projects_by_type_.try_emplace(BaseTypes::StringIdFromStdString(project.id()), idx);
  }

  return true;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadEffects(ErrorsCollection& errors) {
  effect_definitions_.reserve(GetAllEffects().size() + 3 * improvements_by_type_.size() +
                              2 * jobs_by_type_.size() + groups_by_id_.size());

  const auto add_effect_definition = [this,
                                      &errors](const proto::ruleset::effect::Effect& effect_proto) {
    auto definition = std::make_shared<EffectDefinition<BaseTypes>>(effect_proto);
    if (definition->IsBroken()) {
      AddWarning(errors, fmt::format("Failed to load effect {}", effect_proto.id()));
      for (const auto& err : definition->GetLuaErrors()) {
        AddWarning(errors, err);
      }
    }
    effect_definitions_.push_back(
        std::static_pointer_cast<const EffectDefinition<BaseTypes>>(definition));
  };

  const auto add_core_class_dependency = [](proto::ruleset::effect::Code& code) {
    if (std::ranges::find(code.dependencies(), "core.class") == code.dependencies().end()) {
      code.add_dependencies("core.class");
    }
  };

  const auto add_class_effect = [&](std::string id, types::ScopeType scope_type,
                                    const std::string& class_id,
                                    const proto::ruleset::effect::Code& code) {
    proto::ruleset::effect::Effect effect_proto;
    effect_proto.set_id(std::move(id));
    effect_proto.set_scope_type(scope_type);
    effect_proto.mutable_selector()->set_class_(class_id);
    *effect_proto.mutable_effect() = code;
    add_core_class_dependency(*effect_proto.mutable_effect());
    add_effect_definition(effect_proto);
  };

  const auto sorted_entries = [](const auto& values) {
    std::vector<std::pair<std::string, int32_t>> result;
    result.reserve(values.size());
    for (const auto& [id, value] : values) {
      result.emplace_back(id, value);
    }
    std::ranges::sort(result, {}, &std::pair<std::string, int32_t>::first);
    return result;
  };

  for (const auto& effect_proto : GetAllEffects()) {
    add_effect_definition(effect_proto);
  }

  for (const auto& improvement : improvements_.improvements()) {
    if (improvement.has_class_effect()) {
      add_class_effect(fmt::format("{}/class.effect", improvement.id()),
                       types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS, improvement.id(),
                       improvement.class_effect());
    }

    if (!improvement.jobs().empty()) {
      proto::ruleset::effect::Code code;
      std::string lua;
      for (const auto& [job_id, amount] : sorted_entries(improvement.jobs())) {
        lua +=
            fmt::format("target:set_numeric_modifier('job/{}/count', {}, 0.0);\n", job_id, amount);
      }
      code.set_lua(std::move(lua));
      add_class_effect(fmt::format("{}/jobs.effect", improvement.id()),
                       types::ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS, improvement.id(), code);
    }

    if (improvement.has_instance_effect()) {
      proto::ruleset::effect::Effect effect_proto;
      effect_proto.set_id(fmt::format("{}/instance.effect", improvement.id()));
      effect_proto.set_scope_type(types::ScopeType::SCOPE_TYPE_IMPROVEMENT);
      effect_proto.mutable_selector()->set_class_(improvement.id());
      effect_proto.mutable_possible()->set_lua("return true");
      *effect_proto.mutable_effect() = improvement.instance_effect();
      add_effect_definition(effect_proto);
    }
  }

  for (const auto& job : jobs_.jobs()) {
    if (job.has_class_effect()) {
      add_class_effect(fmt::format("{}/class.effect", job.id()),
                       types::ScopeType::SCOPE_TYPE_JOB_CLASS, job.id(), job.class_effect());
    }

    if (!job.input().empty() || !job.output().empty()) {
      proto::ruleset::effect::Code code;
      std::string lua;
      for (const auto& [resource_id, amount] : sorted_entries(job.input())) {
        lua += fmt::format("target:set_numeric_modifier('consumes/@{}', {}, 0.0);\n", resource_id,
                           amount);
      }
      for (const auto& [resource_id, amount] : sorted_entries(job.output())) {
        lua += fmt::format("target:set_numeric_modifier('produces/@{}', {}, 0.0);\n", resource_id,
                           amount);
      }
      code.set_lua(std::move(lua));
      add_class_effect(fmt::format("{}/resources.effect", job.id()),
                       types::ScopeType::SCOPE_TYPE_JOB_CLASS, job.id(), code);
    }
  }

  const auto add_group_effects = [&](const auto& groups, types::ScopeType scope_type) {
    for (const auto& group : groups) {
      if (group.has_group_effect()) {
        add_class_effect(fmt::format("{}/group.effect", group.id()), scope_type, group.id(),
                         group.group_effect());
      }
    }
  };
  add_group_effects(improvements_.improvement_groups(),
                    types::ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP);
  add_group_effects(jobs_.job_groups(), types::ScopeType::SCOPE_TYPE_JOB_GROUP);

  return true;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadVariableDefinitions(ErrorsCollection& errors) {
  if (!parsed_variable_definitions_->SetFixedParameterDomainValues(FixedParameterDomain::kJob,
                                                                   job_ids_)) {
    AddError(errors, "Failed to register job parameter domain values");
    return false;
  }
  if (!parsed_variable_definitions_->SetFixedParameterDomainValues(FixedParameterDomain::kResource,
                                                                   resource_ids_)) {
    AddError(errors, "Failed to register resource parameter domain values");
    return false;
  }

  const auto add_error = [&errors](const auto& definition) {
    AddError(errors,
             fmt::format("Variable {} has undefined type or invalid definition", definition.id()));
  };

  for (int idx = 0; idx < RuleSetBase::GetVariableDefinitions().variables_size(); ++idx) {
    const auto& definition = RuleSetBase::GetVariableDefinitions().variables(idx);
    const auto parsed_definition = VariableDefinitions<BaseTypes>::ParseFromProto(definition);
    const auto variable_id = BaseTypes::StringIdFromStdString(definition.id());

    const auto added = std::visit(
        [&](const auto& parsed) -> std::expected<StringId, ErrorCode> {
          using ParsedT = std::decay_t<decltype(parsed)>;
          if constexpr (std::is_same_v<ParsedT, NumericVariableDefinition<BaseTypes>>) {
            return parsed_variable_definitions_->AddNumericDefinition(variable_id, parsed);
          } else if constexpr (std::is_same_v<ParsedT, StringVariableDefinition<BaseTypes>>) {
            return parsed_variable_definitions_->AddStringDefinition(variable_id, parsed);
          } else if constexpr (std::is_same_v<ParsedT,
                                              ParameterizedNumericVariableDefinition<BaseTypes>>) {
            return parsed_variable_definitions_->AddParameterizedNumericDefinition(parsed);
          } else if constexpr (std::is_same_v<ParsedT,
                                              ParameterizedStringVariableDefinition<BaseTypes>>) {
            return parsed_variable_definitions_->AddParameterizedStringDefinition(parsed);
          } else {
            return std::unexpected(parsed);
          }
        },
        parsed_definition);

    if (!added) {
      add_error(definition);
      return false;
    }
  }

  return true;
}

template <typename BaseTypes>
void RuleSet<BaseTypes>::Clear() {
  RuleSetBase::Clear();
  improvements_by_type_.clear();
  resources_by_id_.clear();
  jobs_by_type_.clear();
  groups_by_id_.clear();
  projects_by_type_.clear();
  resource_ids_.clear();
  job_ids_.clear();
  parsed_variable_definitions_->Clear();
  effect_definitions_.clear();
}

template <typename BaseTypes>
const proto::ruleset::Improvement* RuleSet<BaseTypes>::FindRegionImprovementByType(
    const StringId& improvement_type_id) const {
  auto fit = improvements_by_type_.find(improvement_type_id);
  if (fit != improvements_by_type_.end()) {
    const auto result_idx = fit->second;
    return &improvements_.improvements(result_idx);
  }

  return nullptr;
}

template <typename BaseTypes>
const proto::ruleset::Resource* RuleSet<BaseTypes>::FindResourceByType(
    const StringId& resource_type_id) const {
  auto fit = resources_by_id_.find(resource_type_id);
  if (fit != resources_by_id_.end()) {
    const auto result_idx = fit->second;
    return &resources_.resources(result_idx);
  }

  return nullptr;
}

template <typename BaseTypes>
const proto::ruleset::Job* RuleSet<BaseTypes>::FindJobByType(const StringId& job_type_id) const {
  auto fit = jobs_by_type_.find(job_type_id);
  if (fit != jobs_by_type_.end()) {
    const auto result_idx = fit->second;
    return &jobs_.jobs(result_idx);
  }

  return nullptr;
}

template <typename BaseTypes>
auto RuleSet<BaseTypes>::FindGroupById(const StringId& group_id) const
    -> std::optional<GroupDefinition> {
  const auto found = groups_by_id_.find(group_id);
  if (found == groups_by_id_.end()) {
    return std::nullopt;
  }

  const auto& entry = found->second;
  if (entry.scope_type == types::ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP) {
    return GroupDefinition{&improvements_.improvement_groups(static_cast<int>(entry.index)),
                           entry.scope_type};
  }
  return GroupDefinition{&jobs_.job_groups(static_cast<int>(entry.index)), entry.scope_type};
}

template <typename BaseTypes>
const proto::ruleset::Project* RuleSet<BaseTypes>::FindProjectByType(
    const StringId& project_type_id) const {
  auto fit = projects_by_type_.find(project_type_id);
  if (fit != projects_by_type_.end()) {
    const auto result_idx = fit->second;
    return &projects_.projects(result_idx);
  }

  return nullptr;
}

template <typename BaseTypes>
auto RuleSet<BaseTypes>::ImprovementClassScopeId(StringId civ_id, StringId improvement_class)
    -> StringId {
  return BaseTypes::StringIdFromStdString(
      fmt::format("civ/{}/iclass/{}", civ_id, improvement_class));
}

template <typename BaseTypes>
auto RuleSet<BaseTypes>::JobClassScopeId(StringId civ_id, StringId job_class) -> StringId {
  return BaseTypes::StringIdFromStdString(fmt::format("civ/{}/jclass/{}", civ_id, job_class));
}

template <typename BaseTypes>
auto RuleSet<BaseTypes>::GroupScopeId(StringId civ_id, StringId group_id) -> StringId {
  return BaseTypes::StringIdFromStdString(fmt::format("civ/{}/group/{}", civ_id, group_id));
}

template <typename BaseTypes>
void SerializeTo(const RuleSet<BaseTypes>& source, proto::ruleset::RuleSet& target) {
  target.Clear();
  *target.mutable_improvements() = source.improvements_;
  *target.mutable_biomes() = source.biomes_;
  *target.mutable_resources() = source.resources_;
  *target.mutable_jobs() = source.jobs_;
  *target.mutable_projects() = source.projects_;
  *target.mutable_rendering() = source.rendering_;
  *target.mutable_variables() = source.variable_definitions_;
  for (const auto& effect : source.effects_) {
    *target.add_effects() = effect;
  }
}

template <typename BaseTypes>
RuleSet<BaseTypes> ParseFrom(const proto::ruleset::RuleSet& source,
                             serialize::To<RuleSet<BaseTypes>>) {
  RuleSet<BaseTypes> result;
  result.Clear();
  result.improvements_ = source.improvements();
  result.biomes_ = source.biomes();
  result.resources_ = source.resources();
  result.jobs_ = source.jobs();
  result.projects_ = source.projects();
  result.rendering_ = source.rendering();
  result.variable_definitions_ = source.variables();
  for (const auto& effect : source.effects()) {
    result.effects_.push_back(effect);
  }

  typename RuleSet<BaseTypes>::ErrorsCollection errors;
  bool success = true;
  success &= result.LoadImprovements(errors);
  success &= result.LoadResources(errors);
  success &= result.LoadJobs(errors);
  success &= result.ValidateGroups(errors);
  success &= result.LoadProjects(errors);
  success &= result.LoadVariableDefinitions(errors);
  if (success) {
    success &= result.LoadEffects(errors);
  }
  if (!success) {
    spdlog::warn("Failed to fully restore ruleset from serialized data");
  }

  return result;
}

}  // namespace hs::ruleset
