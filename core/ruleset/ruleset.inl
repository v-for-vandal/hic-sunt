#pragma once

#include "ruleset.hpp"

#include <string>

#include "spdlog/spdlog.h"
namespace hs::ruleset {

namespace {

inline void AddWarning(utils::ErrorsCollection &errors, const std::string &message) {
  spdlog::warn(message);
  errors.AddError({message});
}

inline void AddError(utils::ErrorsCollection &errors, const std::string &message) {
  spdlog::error(message);
  errors.AddError({message});
}

}  // namespace

template <typename BaseTypes>
bool RuleSet<BaseTypes>::Load(const std::vector<std::filesystem::path>& paths,
                              ErrorsCollection &errors) {

  Clear();

  if (!RuleSetBase::Load(paths, errors)) {
    return false;
  }

  bool success = true;
  success &= LoadImprovements(errors);
  success &= LoadResources(errors);
  success &= LoadJobs(errors);
  success &= LoadProjects(errors);
  success &= LoadEffects(errors);
  success &= LoadVariableDefinitions(errors);

  return success;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadImprovements([[maybe_unused]] ErrorsCollection &errors) {
  for (int idx = 0; idx < improvements_.improvements_size(); ++idx) {
    const auto &improvement = improvements_.improvements(idx);
    improvements_by_type_.try_emplace(BaseTypes::StringIdFromStdString(improvement.id()), idx);
  }

  return true;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadResources([[maybe_unused]] ErrorsCollection &errors) {
  for (int idx = 0; idx < resources_.resources_size(); ++idx) {
    const auto &resource = resources_.resources(idx);
    resources_by_id_.try_emplace(BaseTypes::StringIdFromStdString(resource.id()), idx);
  }

  return true;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadJobs(ErrorsCollection &errors) {
  for (int idx = 0; idx < jobs_.jobs_size(); ++idx) {
    const auto &job = jobs_.jobs(idx);
    const auto job_id = BaseTypes::StringIdFromStdString(job.id());
    jobs_by_type_.try_emplace(job_id, idx);

    spdlog::debug("Working with job {}", job_id);

    NumericVariableDefinition<BaseTypes> count_definition;
    count_definition.allowed_scopes.reset();
    count_definition.allowed_scopes |= types::ToScopeTypeFilter(types::ScopeTypeSet::SCOPE_TYPE_SET_JOBS);
    count_definition.minimum = 0;

    auto count_variable_id = fmt::format("job/{}/count", job.id());
    auto add_result = parsed_variable_definitions_->AddNumericDefinition(
        BaseTypes::StringIdFromStdString(count_variable_id), count_definition);
    if (!add_result) {
      AddError(errors,
               fmt::format("Variable {} has conflicting type definition", count_variable_id));
      return false;
    }

    spdlog::debug("Added variable {}", count_variable_id);
    spdlog::debug("Is this variable numeric? {}", GetVariableDefinitions()->IsNumericVariable(BaseTypes::StringIdFromStdString(count_variable_id)));

    for (const auto &[resource_id, resource_idx] : resources_by_id_) {
      (void)resource_idx;

      NumericVariableDefinition<BaseTypes> produces_definition;
      produces_definition.allowed_scopes.reset();
      produces_definition.allowed_scopes |= types::ToScopeTypeFilter(types::ScopeTypeSet::SCOPE_TYPE_SET_JOBS);
      produces_definition.minimum = 0;

      const auto produces_variable_id =
          BaseTypes::StringIdFromStdString(fmt::format("job/{}/produces/{}", job.id(), resource_id));
      add_result = parsed_variable_definitions_->AddNumericDefinition(produces_variable_id,
                                                                      produces_definition);
      if (!add_result) {
        AddError(errors,
                 fmt::format("Variable {} has conflicting type definition",
                     produces_variable_id));
        return false;
      }

      NumericVariableDefinition<BaseTypes> consumes_definition;
      consumes_definition.allowed_scopes.reset();
      consumes_definition.allowed_scopes |= types::ToScopeTypeFilter(types::ScopeTypeSet::SCOPE_TYPE_SET_JOBS);
      consumes_definition.minimum = 0;

      const auto consumes_variable_id =
          BaseTypes::StringIdFromStdString(fmt::format("job/{}/consumes/{}", job.id(), resource_id));
      add_result = parsed_variable_definitions_->AddNumericDefinition(consumes_variable_id,
                                                                      consumes_definition);
      if (!add_result) {
        AddError(errors,
                 fmt::format("Variable {} has conflicting type definition",
                     consumes_variable_id));
        return false;
      }
    }
  }

  return true;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadProjects([[maybe_unused]] ErrorsCollection &errors) {
  for (int idx = 0; idx < projects_.projects_size(); ++idx) {
    const auto &project = projects_.projects(idx);
    projects_by_type_.try_emplace(BaseTypes::StringIdFromStdString(project.id()), idx);
  }

  return true;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadEffects(ErrorsCollection &errors) {
  effect_definitions_.reserve(GetAllEffects().size());
  for (const auto& effect_proto : GetAllEffects()) {
    auto definition = std::make_shared<EffectDefinition<BaseTypes>>(effect_proto);
    if (definition->IsBroken()) {
      AddWarning(errors, fmt::format("Failed to load effect {}", effect_proto.id()));
      for (const auto& err : definition->GetLuaErrors()) {
        AddWarning(errors, err);
      }
    }
    effect_definitions_.push_back(
        std::static_pointer_cast<const EffectDefinition<BaseTypes>>(definition));
  }

  return true;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadVariableDefinitions(ErrorsCollection &errors) {
  for (int idx = 0; idx < RuleSetBase::GetVariableDefinitions().variables_size();
       ++idx) {
    const auto &definition = RuleSetBase::GetVariableDefinitions().variables(idx);
    const auto parsed_definition =
        VariableDefinitions<BaseTypes>::ParseFromProto(definition);
    const auto variable_id = BaseTypes::StringIdFromStdString(definition.id());

    if (const auto *numeric_definition =
            std::get_if<NumericVariableDefinition<BaseTypes>>(&parsed_definition)) {
      auto add_result = parsed_variable_definitions_->AddNumericDefinition(
          variable_id, *numeric_definition);
      if (!add_result) {
        AddError(errors, fmt::format("Variable {} has conflicting type definition",
                                     definition.id()));
        return false;
      }
    } else if (const auto *string_definition =
                   std::get_if<StringVariableDefinition<BaseTypes>>(&parsed_definition)) {
      auto add_result = parsed_variable_definitions_->AddStringDefinition(
          variable_id, *string_definition);
      if (!add_result) {
        AddError(errors, fmt::format("Variable {} has conflicting type definition",
                                     definition.id()));
        return false;
      }
    } else {
      AddError(errors,
               fmt::format("Variable {} has undefined type. It is neither numeric, nor string, nor bool",
                           definition.id()));
    }
  }

  return true;
}

template <typename BaseTypes> void RuleSet<BaseTypes>::Clear() {
  RuleSetBase::Clear();
  improvements_by_type_.clear();
  resources_by_id_.clear();
  jobs_by_type_.clear();
  projects_by_type_.clear();
  parsed_variable_definitions_->Clear();
  effect_definitions_.clear();
}

template <typename BaseTypes>
const proto::ruleset::Improvement *
RuleSet<BaseTypes>::FindRegionImprovementByType(
    const StringId &improvement_type_id) const {
  auto fit = improvements_by_type_.find(improvement_type_id);
  if (fit != improvements_by_type_.end()) {
    const auto result_idx = fit->second;
    return &improvements_.improvements(result_idx);
  }

  return nullptr;
}

template <typename BaseTypes>
const proto::ruleset::Resource *
RuleSet<BaseTypes>::FindResourceByType(const StringId &resource_type_id) const {
  auto fit = resources_by_id_.find(resource_type_id);
  if (fit != resources_by_id_.end()) {
    const auto result_idx = fit->second;
    return &resources_.resources(result_idx);
  }

  return nullptr;
}

template <typename BaseTypes>
const proto::ruleset::Job *
RuleSet<BaseTypes>::FindJobByType(const StringId &job_type_id) const {
  auto fit = jobs_by_type_.find(job_type_id);
  if (fit != jobs_by_type_.end()) {
    const auto result_idx = fit->second;
    return &jobs_.jobs(result_idx);
  }

  return nullptr;
}

template <typename BaseTypes>
const proto::ruleset::Project *
RuleSet<BaseTypes>::FindProjectByType(const StringId &project_type_id) const {
  auto fit = projects_by_type_.find(project_type_id);
  if (fit != projects_by_type_.end()) {
    const auto result_idx = fit->second;
    return &projects_.projects(result_idx);
  }

  return nullptr;
}

} // namespace hs::ruleset
