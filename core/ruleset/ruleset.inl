#include <system_error>
#include "ruleset.hpp"

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

  if (!RuleSetBase::Load(paths, errors)) {
    return false;
  }

  bool success = true;
  success &= LoadImprovements(errors);
  success &= LoadJobs(errors);
  success &= LoadProjects(errors);
  success &= LoadEffects(errors);
  success &= LoadVariableDefinitions(errors);

  return success;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadImprovements([[maybe_unused]] ErrorsCollection &errors) {
  improvements_by_type_.clear();
  for (int idx = 0; idx < improvements_.improvements_size(); ++idx) {
    const auto &improvement = improvements_.improvements(idx);
    improvements_by_type_.try_emplace(BaseTypes::StringIdFromStdString(improvement.id()), idx);
  }

  return true;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadJobs([[maybe_unused]] ErrorsCollection &errors) {
  jobs_by_type_.clear();
  for (int idx = 0; idx < jobs_.jobs_size(); ++idx) {
    const auto &job = jobs_.jobs(idx);
    jobs_by_type_.try_emplace(BaseTypes::StringIdFromStdString(job.id()), idx);
  }

  return true;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadProjects([[maybe_unused]] ErrorsCollection &errors) {
  projects_by_type_.clear();
  for (int idx = 0; idx < projects_.projects_size(); ++idx) {
    const auto &project = projects_.projects(idx);
    projects_by_type_.try_emplace(BaseTypes::StringIdFromStdString(project.id()), idx);
  }

  return true;
}

template <typename BaseTypes>
bool RuleSet<BaseTypes>::LoadEffects(ErrorsCollection &errors) {
  effect_definitions_.clear();
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
  parsed_variable_definitions_->Clear();
  for (int idx = 0; idx < RuleSetBase::GetVariableDefinitions().variables_size();
       ++idx) {
    const auto &definition = RuleSetBase::GetVariableDefinitions().variables(idx);

    if (definition.has_numeric()) {
      const auto &numeric = definition.numeric();
      NumericVariableDefinition<BaseTypes> numeric_definition;
      if (numeric.has_minimum()) {
        numeric_definition.minimum = numeric.minimum();
      }
      if (numeric.has_maximum()) {
        numeric_definition.maximum = numeric.maximum();
      }
      auto add_result = parsed_variable_definitions_->AddNumericDefinition(
          BaseTypes::StringIdFromStdString(definition.id()),
          numeric_definition);
      if (!add_result) {
        AddError(errors, fmt::format("Variable {} has conflicting type definition",
                                     definition.id()));
        return false;
      }
    } else if (definition.has_string()) {
      const auto &string_ = definition.string();
      StringVariableDefinition<BaseTypes> string_definition;
      if (!string_.default_().empty()) {
        string_definition.default_value =
            BaseTypes::StringIdFromStdString(string_.default_());
      }
      auto add_result = parsed_variable_definitions_->AddStringDefinition(
          BaseTypes::StringIdFromStdString(definition.id()),
          string_definition);
      if (!add_result) {
        AddError(errors, fmt::format("Variable {} has conflicting type definition",
                                     definition.id()));
        return false;
      }
    } else if (definition.has_boolean()) {
      NumericVariableDefinition<BaseTypes> numeric_definition;
      numeric_definition.minimum = 0;
      numeric_definition.maximum = 1;
      auto add_result = parsed_variable_definitions_->AddNumericDefinition(
          BaseTypes::StringIdFromStdString(definition.id()),
          numeric_definition);
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
  jobs_by_type_.clear();
  projects_by_type_.clear();
  parsed_variable_definitions_->Clear();
  effect_definitions_.clear();
}

template <typename BaseTypes>
const proto::ruleset::RegionImprovement *
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
