#include "ruleset.hpp"

#include "spdlog/spdlog.h"
namespace hs::ruleset {

template <typename BaseTypes>
bool RuleSet<BaseTypes>::Load(const std::vector<std::filesystem::path>& paths,
                              ErrorsCollection &errors) {

  if (!RuleSetBase::Load(paths, errors)) {
    return false;
  }

  // Building hashes
  for (int idx = 0; idx < improvements_.improvements_size(); ++idx) {
    const auto &improvement = improvements_.improvements(idx);
    improvements_by_type_.try_emplace(BaseTypes::StringIdFromStdString(improvement.id()), idx);
  }

  for (int idx = 0; idx < jobs_.jobs_size(); ++idx) {
    const auto &job = jobs_.jobs(idx);
    jobs_by_type_.try_emplace(BaseTypes::StringIdFromStdString(job.id()), idx);
  }

  for (int idx = 0; idx < projects_.projects_size(); ++idx) {
    const auto &project = projects_.projects(idx);
    projects_by_type_.try_emplace(BaseTypes::StringIdFromStdString(project.id()), idx);
  }

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
      parsed_variable_definitions_.AddNumericDefinition(
          BaseTypes::StringIdFromStdString(definition.id()),
          numeric_definition);
    } else if (definition.has_string()) {
      const auto &string_ = definition.string();
      StringVariableDefinition<BaseTypes> string_definition;
      if (!string_.default_().empty()) {
        string_definition.default_value =
            BaseTypes::StringIdFromStdString(string_.default_());
      }
      parsed_variable_definitions_.AddStringDefinition(
          BaseTypes::StringIdFromStdString(definition.id()),
          string_definition);
    } else if (definition.has_boolean()) {
      NumericVariableDefinition<BaseTypes> numeric_definition;
      numeric_definition.minimum = 0;
      numeric_definition.maximum = 1;
      parsed_variable_definitions_.AddNumericDefinition(
          BaseTypes::StringIdFromStdString(definition.id()),
          numeric_definition);
    } else {
      SPDLOG_ERROR(
          "Variable {} has undefined type. It is neither numeric, nor string, nor bool",
          definition.id());
    }
  }

  return true;
}

template <typename BaseTypes> void RuleSet<BaseTypes>::Clear() {
  RuleSetBase::Clear();
  improvements_by_type_.clear();
  jobs_by_type_.clear();
  projects_by_type_.clear();
  parsed_variable_definitions_.Clear();
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
