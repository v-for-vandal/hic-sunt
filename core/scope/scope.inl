#pragma once

#include <core/utils/serialize.hpp>

#include "core/ruleset/variable_definition.hpp"
#include "core/types/error_code.hpp"
#include "scope.hpp"
#include "spdlog/spdlog.h"

namespace hs::scope {

template <typename BaseTypes>
Scope<BaseTypes>::Scope(StringId id, types::ScopeType scope_type)
    : id_(id), scope_type_(scope_type) {
  if (BaseTypes::IsNullToken(id_)) {
    const uint64_t address_as_uint = reinterpret_cast<uint64_t>(this);
    id_ = BaseTypes::StringIdFromStdString(fmt::format("{:x}", address_as_uint));
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::SetVariableDefinitions(const VariableDefinitionsConstPtr& definitions) {
  if (scope_type_ != types::ScopeType::SCOPE_TYPE_WORLD) {
    spdlog::warn("Setting definitiosn on non-root scope is not recommended");
  }
  cached_definitions_ = definitions;
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillNumericModifiers(const StringId& variable,
                                            NumericQueryAccumulator& accumulator,
                                            VisitedScopes& visited) const {
  FillNumericModifiers(variable, variable, accumulator, visited);
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillNumericModifiers(const StringId& variable,
                                            const StringId& normalized_variable,
                                            NumericQueryAccumulator& accumulator,
                                            VisitedScopes& visited) const {
  if (!visited.insert(this).second) {
    return;
  }

  if (variable != normalized_variable) {
    if (auto it = numeric_variables_.find(normalized_variable); it != numeric_variables_.end()) {
      it->second.CalculateModifiers(accumulator.add, accumulator.mult);
    }
  }

  if (auto it = numeric_variables_.find(variable); it != numeric_variables_.end()) {
    it->second.CalculateModifiers(accumulator.add, accumulator.mult);
  }

  if (parent_ != nullptr) {
    parent_->FillNumericModifiers(variable, normalized_variable, accumulator, visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
    if (tag_scope != nullptr) {
      tag_scope->FillNumericModifiers(variable, normalized_variable, accumulator, visited);
    }
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillStringModifiers(const StringId& variable,
                                           StringQueryAccumulator& accumulator,
                                           VisitedScopes& visited) {
  FillStringModifiers(variable, variable, accumulator, visited);
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillStringModifiers(const StringId& variable,
                                           const StringId& normalized_variable,
                                           StringQueryAccumulator& accumulator,
                                           VisitedScopes& visited) {
  if (!visited.insert(this).second) {
    return;
  }

  if (variable != normalized_variable) {
    if (auto fit = string_variables_.find(normalized_variable); fit != string_variables_.end()) {
      fit->second.MergeModifiers(accumulator.value, accumulator.level, accumulator.key);
    }
  }

  if (auto fit = string_variables_.find(variable); fit != string_variables_.end()) {
    fit->second.MergeModifiers(accumulator.value, accumulator.level, accumulator.key);
  }

  // get parent value
  if (parent_ != nullptr) {
    parent_->FillStringModifiers(variable, normalized_variable, accumulator, visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
    tag_scope->FillStringModifiers(variable, normalized_variable, accumulator, visited);
  }
}

template <typename BaseTypes>
auto Scope<BaseTypes>::CalculateNumericValue(const NumericVariableDefinition& variable_definition,
                                             const ParsedVariableQuery& variable_query) const
    -> NumericValue {
  NumericQueryAccumulator accumulator;
  const auto& normalized_id =
      variable_query.is_parameterized ? variable_query.normalized_id : variable_definition.id;

  VisitedScopes visited;
  FillNumericModifiers(variable_definition.id, normalized_id, accumulator, visited);

  accumulator.mult = 1 + accumulator.mult;
  accumulator.mult = std::max<NumericValue>(accumulator.mult, 0);

  auto value = accumulator.add * accumulator.mult;
  value = std::clamp(value, variable_definition.minimum, variable_definition.maximum);
  return value;
}

template <typename BaseTypes>
auto Scope<BaseTypes>::CalculateStringValue(const StringVariableDefinition& variable_definition,
                                            const ParsedVariableQuery& variable_query) -> StringId {
  StringQueryAccumulator accumulator{.value = variable_definition.default_value};
  const auto& normalized_id =
      variable_query.is_parameterized ? variable_query.normalized_id : variable_definition.id;

  VisitedScopes visited;
  FillStringModifiers(variable_definition.id, normalized_id, accumulator, visited);

  return accumulator.value;
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetNumericValue(const StringId& variable)
    -> std::expected<Scope::NumericValue, ErrorCode> {
  auto parsed = ruleset::ParseVariableQuery<BaseTypes>(variable, false);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  return GetNumericValue(*parsed);
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetNumericValue(const ParsedVariableQuery& variable)
    -> std::expected<Scope::NumericValue, ErrorCode> {
  if (variable.has_wildcard) {
    return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
  }

  auto vardef = GetVariableDefinitions()->FindNumericVariable(variable);
  if (!vardef) {
    return std::unexpected(vardef.error());
  }

  return CalculateNumericValue(*vardef, variable);
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetStringValue(const StringId& variable)
    -> std::expected<Scope::StringId, ErrorCode> {
  auto parsed = ruleset::ParseVariableQuery<BaseTypes>(variable, false);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  return GetStringValue(*parsed);
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetStringValue(const ParsedVariableQuery& variable)
    -> std::expected<Scope::StringId, ErrorCode> {
  if (variable.has_wildcard) {
    return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
  }

  auto vardef = GetVariableDefinitions()->FindStringVariable(variable);
  if (!vardef) {
    return std::unexpected(vardef.error());
  }

  return CalculateStringValue(*vardef, variable);
}

template <typename BaseTypes>
bool Scope<BaseTypes>::IsStringVariable(const StringId& variable) const {
  return GetVariableDefinitions()->GetVariableType(variable) ==
         ruleset::VariableDefinitions<BaseTypes>::VariableType::kString;
}

template <typename BaseTypes>
bool Scope<BaseTypes>::IsStringVariable(const ParsedVariableQuery& variable) const {
  return GetVariableDefinitions()->GetVariableType(variable) ==
         ruleset::VariableDefinitions<BaseTypes>::VariableType::kString;
}

template <typename BaseTypes>
bool Scope<BaseTypes>::IsNumericVariable(const StringId& variable) const {
  return GetVariableDefinitions()->GetVariableType(variable) ==
         ruleset::VariableDefinitions<BaseTypes>::VariableType::kNumeric;
}

template <typename BaseTypes>
bool Scope<BaseTypes>::IsNumericVariable(const ParsedVariableQuery& variable) const {
  return GetVariableDefinitions()->GetVariableType(variable) ==
         ruleset::VariableDefinitions<BaseTypes>::VariableType::kNumeric;
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetNumericQuery(const StringId& query)
    -> std::expected<std::vector<NumericQueryResult>, ErrorCode> {
  auto parsed = ruleset::ParseVariableQuery<BaseTypes>(query, true);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  return GetNumericQuery(*parsed);
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetNumericQuery(const ParsedVariableQuery& query)
    -> std::expected<std::vector<NumericQueryResult>, ErrorCode> {
  if (!query.has_wildcard) {
    auto variable_definition = GetVariableDefinitions()->FindNumericVariable(query);
    if (!variable_definition) {
      return std::unexpected(variable_definition.error());
    }
    return std::vector<NumericQueryResult>{
        {variable_definition->id, CalculateNumericValue(*variable_definition, query)}};
  }

  if (!query.is_parameterized) {
    return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
  }

  auto parameterized_definition =
      GetVariableDefinitions()->FindParameterizedNumericDefinition(query);
  if (!parameterized_definition) {
    return std::unexpected(parameterized_definition.error());
  }

  NumericQueryAccumulator default_accumulator;
  absl::flat_hash_map<StringId, NumericQueryAccumulator> values;
  VisitedScopes visited;
  DoCollectNumericQueryResults(query, default_accumulator, values, visited);

  std::vector<NumericQueryResult> result;
  result.reserve(values.size());
  for (auto [id, modifiers] : values) {
    modifiers.add += default_accumulator.add;
    modifiers.mult += default_accumulator.mult;
    auto mult = 1 + modifiers.mult;
    mult = std::max<NumericValue>(mult, 0);
    auto value = modifiers.add * mult;
    value = std::clamp(value, parameterized_definition->minimum, parameterized_definition->maximum);
    result.emplace_back(id, value);
  }
  return result;
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetStringQuery(const StringId& query)
    -> std::expected<std::vector<StringQueryResult>, ErrorCode> {
  auto parsed = ruleset::ParseVariableQuery<BaseTypes>(query, true);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  return GetStringQuery(*parsed);
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetStringQuery(const ParsedVariableQuery& query)
    -> std::expected<std::vector<StringQueryResult>, ErrorCode> {
  if (!query.has_wildcard) {
    auto variable_definition = GetVariableDefinitions()->FindStringVariable(query);
    if (!variable_definition) {
      return std::unexpected(variable_definition.error());
    }
    return std::vector<StringQueryResult>{
        {variable_definition->id, CalculateStringValue(*variable_definition, query)}};
  }

  if (!query.is_parameterized) {
    return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
  }

  auto parameterized_definition =
      GetVariableDefinitions()->FindParameterizedStringDefinition(query);
  if (!parameterized_definition) {
    return std::unexpected(parameterized_definition.error());
  }

  StringQueryAccumulator default_accumulator{.value = parameterized_definition->default_value};
  absl::flat_hash_map<StringId, StringQueryAccumulator> values;
  VisitedScopes visited;
  DoCollectStringQueryResults(query, default_accumulator, values, visited);

  std::vector<StringQueryResult> result;
  result.reserve(values.size());
  for (auto [id, accumulator] : values) {
    if ((default_accumulator.level > accumulator.level) ||
        (default_accumulator.level == accumulator.level &&
         default_accumulator.key > accumulator.key)) {
      accumulator = default_accumulator;
    }
    result.emplace_back(id, accumulator.value);
  }
  return result;
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::ValidateNumericModifierTarget(
    const StringId& variable, const StringId& key) const {
  if (BaseTypes::IsNullToken(key)) {
    return std::unexpected(ErrorCode::ERR_EMPTY_MODIFIER_KEY);
  }

  auto parsed = ruleset::ParseVariableQuery<BaseTypes>(variable, true);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  if (parsed->has_wildcard) {
    return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
  }

  const auto& definitions = GetVariableDefinitions();
  if (definitions->IsEmpty()) {
    return {};
  }

  const auto variable_definition = definitions->FindNumericVariable(variable);
  if (!variable_definition) {
    return std::unexpected(variable_definition.error());
  }

  if (!variable_definition->allowed_scopes[scope_type_]) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_SCOPE_TYPE);
  }

  return {};
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::ValidateStringModifierTarget(
    const StringId& variable, const StringId& key) const {
  if (BaseTypes::IsNullToken(key)) {
    return std::unexpected(ErrorCode::ERR_EMPTY_MODIFIER_KEY);
  }

  auto parsed = ruleset::ParseVariableQuery<BaseTypes>(variable, true);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  if (parsed->has_wildcard) {
    return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
  }

  const auto& definitions = GetVariableDefinitions();
  if (definitions->IsEmpty()) {
    return {};
  }

  const auto variable_definition = definitions->FindStringVariable(variable);
  if (!variable_definition) {
    return std::unexpected(variable_definition.error());
  }

  if (!variable_definition->allowed_scopes[scope_type_]) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_SCOPE_TYPE);
  }

  return {};
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::SetNumericModifier(const StringId& variable,
                                                                    const StringId& key,
                                                                    NumericValue add,
                                                                    NumericValue mult,
                                                                    size_t modificationTime) {
  auto validation = ValidateNumericModifierTarget(variable, key);
  if (!validation) {
    return validation;
  }
  return numeric_variables_[variable].SetModifier(key, add, mult, modificationTime);
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::ChangeNumericModifier(const StringId& variable,
                                                                       const StringId& key,
                                                                       NumericValue add,
                                                                       NumericValue mult,
                                                                       size_t modificationTime) {
  auto validation = ValidateNumericModifierTarget(variable, key);
  if (!validation) {
    return validation;
  }
  return numeric_variables_[variable].ChangeModifier(key, add, mult, modificationTime);
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::SetStringModifier(const StringId& variable,
                                                                   const StringId& key,
                                                                   const StringId& value,
                                                                   NumericValue level,
                                                                   size_t modificationTime) {
  auto validation = ValidateStringModifierTarget(variable, key);
  if (!validation) {
    return validation;
  }
  return string_variables_[variable].SetModifier(key, value, level, modificationTime);
}

template <typename BaseTypes>
std::expected<size_t, ErrorCode> Scope<BaseTypes>::GetModificationTime(
    const StringId& variable) const {
  auto parsed = ruleset::ParseVariableQuery<BaseTypes>(variable, true);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  return GetModificationTime(*parsed);
}

template <typename BaseTypes>
std::expected<size_t, ErrorCode> Scope<BaseTypes>::GetModificationTime(
    const ParsedVariableQuery& variable) const {
  if (!variable.has_wildcard) {
    auto vardef = GetVariableDefinitions()->FindVariable(variable);
    if (!vardef) {
      return std::unexpected(vardef.error());
    }

    VisitedScopes visited;
    size_t modification_time = DoGetModificationTime(*vardef, visited);
    if (variable.is_parameterized && variable.normalized_id != vardef->id) {
      VisitedScopes normalized_visited;
      modification_time = std::max(
          modification_time, DoGetModificationTime(variable.normalized_id, normalized_visited));
    }
    return modification_time;
  }

  if (!variable.is_parameterized) {
    return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
  }

  absl::flat_hash_set<StringId> ids;
  VisitedScopes collect_visited;
  if (GetVariableDefinitions()->GetParameterizedNumericDefinitions().contains(
          variable.normalized_id)) {
    DoCollectNumericMaterializedIds(variable, ids, collect_visited);
  } else if (GetVariableDefinitions()->GetParameterizedStringDefinitions().contains(
                 variable.normalized_id)) {
    DoCollectStringMaterializedIds(variable, ids, collect_visited);
  } else {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  VisitedScopes normalized_visited;
  size_t modification_time = DoGetModificationTime(variable.normalized_id, normalized_visited);
  for (const auto& id : ids) {
    VisitedScopes visited;
    modification_time = std::max(modification_time, DoGetModificationTime(id, visited));
  }
  return modification_time;
}

template <typename BaseTypes>
size_t Scope<BaseTypes>::DoGetModificationTime(const VariableDefinitionBase& variable_definition,
                                               VisitedScopes& visited) const {
  if (!visited.insert(this).second) {
    return 0;
  }

  size_t modification_time = 0;

  if (auto fit = numeric_variables_.find(variable_definition.id); fit != numeric_variables_.end()) {
    modification_time = std::max(modification_time, fit->second.GetModificationTime());
  }

  if (auto fit = string_variables_.find(variable_definition.id); fit != string_variables_.end()) {
    modification_time = std::max(modification_time, fit->second.GetModificationTime());
  }

  if (parent_ != nullptr) {
    auto parent_modification_time = parent_->DoGetModificationTime(variable_definition, visited);
    modification_time = std::max(modification_time, parent_modification_time);
  }

  for (const auto& tag_scope : tag_scopes_) {
    auto tag_modification_time = tag_scope->DoGetModificationTime(variable_definition, visited);
    modification_time = std::max(modification_time, tag_modification_time);
  }

  return modification_time;
}

template <typename BaseTypes>
size_t Scope<BaseTypes>::DoGetModificationTime(const StringId& variable,
                                               VisitedScopes& visited) const {
  if (!visited.insert(this).second) {
    return 0;
  }

  size_t modification_time = 0;

  if (auto fit = numeric_variables_.find(variable); fit != numeric_variables_.end()) {
    modification_time = std::max(modification_time, fit->second.GetModificationTime());
  }

  if (auto fit = string_variables_.find(variable); fit != string_variables_.end()) {
    modification_time = std::max(modification_time, fit->second.GetModificationTime());
  }

  if (parent_ != nullptr) {
    modification_time =
        std::max(modification_time, parent_->DoGetModificationTime(variable, visited));
  }

  for (const auto& tag_scope : tag_scopes_) {
    modification_time =
        std::max(modification_time, tag_scope->DoGetModificationTime(variable, visited));
  }

  return modification_time;
}

template <typename BaseTypes>
void Scope<BaseTypes>::DoCollectNumericMaterializedIds(const ParsedVariableQuery& query,
                                                       absl::flat_hash_set<StringId>& ids,
                                                       VisitedScopes& visited) const {
  if (!visited.insert(this).second) {
    return;
  }

  for (const auto& [variable_id, _] : numeric_variables_) {
    const auto parsed = ruleset::ParseVariableQuery<BaseTypes>(variable_id, false);
    if (parsed && ruleset::details::QueryMatchesConcreteId(query, *parsed)) {
      ids.insert(variable_id);
    }
  }

  if (parent_ != nullptr) {
    parent_->DoCollectNumericMaterializedIds(query, ids, visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
    tag_scope->DoCollectNumericMaterializedIds(query, ids, visited);
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::DoCollectStringMaterializedIds(const ParsedVariableQuery& query,
                                                      absl::flat_hash_set<StringId>& ids,
                                                      VisitedScopes& visited) const {
  if (!visited.insert(this).second) {
    return;
  }

  for (const auto& [variable_id, _] : string_variables_) {
    const auto parsed = ruleset::ParseVariableQuery<BaseTypes>(variable_id, false);
    if (parsed && ruleset::details::QueryMatchesConcreteId(query, *parsed)) {
      ids.insert(variable_id);
    }
  }

  if (parent_ != nullptr) {
    parent_->DoCollectStringMaterializedIds(query, ids, visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
    tag_scope->DoCollectStringMaterializedIds(query, ids, visited);
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::DoCollectNumericQueryResults(
    const ParsedVariableQuery& query, NumericQueryAccumulator& default_accumulator,
    absl::flat_hash_map<StringId, NumericQueryAccumulator>& result, VisitedScopes& visited) const {
  if (!visited.insert(this).second) {
    return;
  }

  for (const auto& [variable_id, variable] : numeric_variables_) {
    if (variable_id == query.normalized_id) {
      variable.CalculateModifiers(default_accumulator.add, default_accumulator.mult);
      continue;
    }

    const auto parsed = ruleset::ParseVariableQuery<BaseTypes>(variable_id, false);
    if (!parsed || !ruleset::details::QueryMatchesConcreteId(query, *parsed)) {
      continue;
    }

    auto [it, _] = result.try_emplace(variable_id);
    variable.CalculateModifiers(it->second.add, it->second.mult);
  }

  if (parent_ != nullptr) {
    parent_->DoCollectNumericQueryResults(query, default_accumulator, result, visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
    tag_scope->DoCollectNumericQueryResults(query, default_accumulator, result, visited);
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::DoCollectStringQueryResults(
    const ParsedVariableQuery& query, StringQueryAccumulator& default_accumulator,
    absl::flat_hash_map<StringId, StringQueryAccumulator>& result, VisitedScopes& visited) const {
  if (!visited.insert(this).second) {
    return;
  }

  for (const auto& [variable_id, variable] : string_variables_) {
    if (variable_id == query.normalized_id) {
      variable.MergeModifiers(default_accumulator.value, default_accumulator.level,
                              default_accumulator.key);
      continue;
    }

    const auto parsed = ruleset::ParseVariableQuery<BaseTypes>(variable_id, false);
    if (!parsed || !ruleset::details::QueryMatchesConcreteId(query, *parsed)) {
      continue;
    }

    auto [it, _] = result.try_emplace(variable_id);
    variable.MergeModifiers(it->second.value, it->second.level, it->second.key);
  }

  if (parent_ != nullptr) {
    parent_->DoCollectStringQueryResults(query, default_accumulator, result, visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
    tag_scope->DoCollectStringQueryResults(query, default_accumulator, result, visited);
  }
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetVariableDefinitions() const -> const VariableDefinitionsConstPtr& {
  if (cached_definitions_->IsEmpty()) {
    if (parent_) {
      cached_definitions_ = parent_->GetVariableDefinitions();
    } else {
      spdlog::warn("Can not find non-empty variable definitions because scope {} has no parent",
                   id_);
    }
  }

  return cached_definitions_;
}

template <typename BaseTypes>
void Scope<BaseTypes>::ExplainNumericVariable(const StringId& variable, auto&& collect_fn) {
  VisitedScopes visited;
  DoExplainNumericVariable(variable, std::forward<decltype(collect_fn)>(collect_fn), visited);
}

template <typename BaseTypes>
template <typename CollectFn>
void Scope<BaseTypes>::DoExplainNumericVariable(const StringId& variable, CollectFn&& collect_fn,
                                                VisitedScopes& visited) {
  if (!visited.insert(this).second) {
    return;
  }

  auto it = numeric_variables_.find(variable);
  if (it != numeric_variables_.end()) {
    it->second.ExplainModifiers([this, &collect_fn, &variable](
                                    const StringId& modifier, NumericValue add, NumericValue mult) {
      collect_fn(this->id_, variable, modifier, add, mult);
    });
  }

  if (parent_ != nullptr) {
    parent_->DoExplainNumericVariable(variable, std::forward<CollectFn>(collect_fn), visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
    tag_scope->DoExplainNumericVariable(variable, std::forward<CollectFn>(collect_fn), visited);
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::ExplainStringVariable(const StringId& variable, auto&& collect_fn) {
  VisitedScopes visited;
  DoExplainStringVariable(variable, std::forward<decltype(collect_fn)>(collect_fn), visited);
}

template <typename BaseTypes>
template <typename CollectFn>
void Scope<BaseTypes>::DoExplainStringVariable(const StringId& variable, CollectFn&& collect_fn,
                                               VisitedScopes& visited) {
  if (!visited.insert(this).second) {
    return;
  }

  auto it = string_variables_.find(variable);
  if (it != string_variables_.end()) {
    it->second.ExplainModifiers([this, &collect_fn, &variable](const StringId& modifier,
                                                               const StringId& value,
                                                               NumericValue level) {
      collect_fn(this->id_, variable, modifier, value, level);
    });
  }

  if (parent_ != nullptr) {
    parent_->DoExplainStringVariable(variable, std::forward<CollectFn>(collect_fn), visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
    tag_scope->DoExplainStringVariable(variable, std::forward<CollectFn>(collect_fn), visited);
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::ExplainAllVariables(auto&& collect_fn) {
  // TODO: Replace empty id with hex-string for address
  for (const auto& [k, v] : numeric_variables_) {
    v.ExplainModifiers(
        [this, &collect_fn, &k](const StringId& modifier, NumericValue add, NumericValue mult) {
          collect_fn(this->id_, k, modifier, add, mult);
        });
  }
  for (const auto& [k, v] : string_variables_) {
    v.ExplainModifiers([this, &collect_fn, &k](const StringId& modifier, const StringId& value,
                                               NumericValue level) {
      collect_fn(this->id_, k, modifier, value, level);
    });
  }
  if (parent_ != nullptr) {
    parent_->ExplainAllVariables(collect_fn);
  }
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::AddTagLink(
    [[maybe_unused]] const StringId& tag_name, const ScopePtr& tag_scope) {
  if (!hs::types::CanTagLinkScopes(scope_type_, tag_scope->scope_type_)) {
    spdlog::warn("Scope of type {} can not be tag-linked to of scope of type {}", scope_type_,
                 tag_scope->scope_type_);
    return std::unexpected(ErrorCode::ERR_INCORRECT_SCOPE_TYPE);
  }

  for (const auto& existing_scope : tag_scopes_) {
    if (existing_scope->GetId() == tag_scope->GetId()) {
      spdlog::error("Scope {} already has tag link to scope {}", id_, tag_scope->GetId());
      return std::unexpected(ErrorCode::ERR_SCOPE_ALREADY_EXISTS);
    }
  }

  tag_scopes_.push_back(tag_scope);
  return {};
}

template <typename BaseTypes>
void Scope<BaseTypes>::ClearCache() {
  cached_definitions_.reset();
}

template <typename BaseTypes>
void Scope<BaseTypes>::RestoreTagLinks(
    const absl::flat_hash_map<StringId, ScopePtr>& scopes_by_id) {
  tag_scopes_.clear();
  tag_scopes_.reserve(pending_tag_scope_ids_.size());
  for (const auto& scope_id : pending_tag_scope_ids_) {
    auto fit = scopes_by_id.find(scope_id);
    if (fit == scopes_by_id.end()) {
      spdlog::warn("Can not restore tag link from scope {} to missing scope {}", id_, scope_id);
      continue;
    }
    tag_scopes_.push_back(fit->second);
  }
  pending_tag_scope_ids_.clear();
}

template <typename BaseTypes>
void SerializeTo(const Scope<BaseTypes>& source, proto::scope::Scope& target) {
  target.Clear();
  target.set_id(BaseTypes::ToProtoString(source.id_));
  target.set_scope_type(source.scope_type_);

  for (const auto& [variable_id, variable] : source.numeric_variables_) {
    auto* variable_proto = target.add_numeric_variables();
    variable_proto->set_id(BaseTypes::ToProtoString(variable_id));
    variable_proto->set_modification_time(variable.GetModificationTime());
    variable.ExplainModifiers([variable_proto](const auto& key, auto add, auto mult) {
      auto* modifier_proto = variable_proto->add_modifiers();
      modifier_proto->set_key(BaseTypes::ToProtoString(key));
      modifier_proto->set_add(add);
      modifier_proto->set_mult(mult);
    });
  }

  for (const auto& [variable_id, variable] : source.string_variables_) {
    auto* variable_proto = target.add_string_variables();
    variable_proto->set_id(BaseTypes::ToProtoString(variable_id));
    variable_proto->set_modification_time(variable.GetModificationTime());
    variable.ExplainModifiers([variable_proto](const auto& key, const auto& value, auto level) {
      auto* modifier_proto = variable_proto->add_modifiers();
      modifier_proto->set_key(BaseTypes::ToProtoString(key));
      modifier_proto->set_value(BaseTypes::ToProtoString(value));
      modifier_proto->set_level(level);
    });
  }

  for (const auto& tag_scope : source.tag_scopes_) {
    if (tag_scope != nullptr) {
      target.add_tag_scope_ids(BaseTypes::ToProtoString(tag_scope->GetId()));
    }
  }
  for (const auto& tag_scope_id : source.pending_tag_scope_ids_) {
    target.add_tag_scope_ids(BaseTypes::ToProtoString(tag_scope_id));
  }
}

template <typename BaseTypes>
Scope<BaseTypes> ParseFrom(const proto::scope::Scope& scope, serialize::To<Scope<BaseTypes>>) {
  using StringId = typename BaseTypes::StringId;
  Scope<BaseTypes> result;
  result.id_ = ParseFrom(scope.id(), serialize::To<StringId>{});
  result.scope_type_ = scope.scope_type();

  for (const auto& variable_proto : scope.numeric_variables()) {
    auto variable_id = ParseFrom(variable_proto.id(), serialize::To<StringId>{});
    auto& variable = result.numeric_variables_[variable_id];
    for (const auto& modifier_proto : variable_proto.modifiers()) {
      auto key = ParseFrom(modifier_proto.key(), serialize::To<StringId>{});
      auto set_result = variable.SetModifier(key, modifier_proto.add(), modifier_proto.mult(),
                                             variable_proto.modification_time());
      if (!set_result) {
        spdlog::warn("Failed to restore numeric modifier {} for variable {} in scope {}", key,
                     variable_id, result.id_);
      }
    }
  }

  for (const auto& variable_proto : scope.string_variables()) {
    auto variable_id = ParseFrom(variable_proto.id(), serialize::To<StringId>{});
    auto& variable = result.string_variables_[variable_id];
    for (const auto& modifier_proto : variable_proto.modifiers()) {
      auto key = ParseFrom(modifier_proto.key(), serialize::To<StringId>{});
      auto value = ParseFrom(modifier_proto.value(), serialize::To<StringId>{});
      auto set_result = variable.SetModifier(key, value, modifier_proto.level(),
                                             variable_proto.modification_time());
      if (!set_result) {
        spdlog::warn("Failed to restore string modifier {} for variable {} in scope {}", key,
                     variable_id, result.id_);
      }
    }
  }

  result.pending_tag_scope_ids_.reserve(scope.tag_scope_ids_size());
  for (const auto& tag_scope_id : scope.tag_scope_ids()) {
    result.pending_tag_scope_ids_.push_back(ParseFrom(tag_scope_id, serialize::To<StringId>{}));
  }

  return result;
}

}  // namespace hs::scope
