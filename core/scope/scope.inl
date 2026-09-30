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
void Scope<BaseTypes>::FillNumericModifiers(const NumericVariableDefinition& variable_definition,
                                            NumericValue& add, NumericValue& mult,
                                            VisitedScopes& visited) const {
  if (!visited.insert(this).second) {
    return;
  }

  if (auto it = numeric_variables_.find(variable_definition.id); it != numeric_variables_.end()) {
    it->second.CalculateModifiers(add, mult);
  }

  if (parent_ != nullptr) {
    parent_->FillNumericModifiers(variable_definition, add, mult, visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
    if (tag_scope != nullptr) {
      tag_scope->FillNumericModifiers(variable_definition, add, mult, visited);
    }
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillStringModifiers(const StringVariableDefinition& variable_definition,
                                           StringId& value, NumericValue& level,
                                           VisitedScopes& visited) {
  if (!visited.insert(this).second) {
    return;
  }

  if (auto fit = string_variables_.find(variable_definition.id); fit != string_variables_.end()) {
    fit->second.CalculateModifiers(value, level);
  }

  // get parent value
  if (parent_ != nullptr) {
    parent_->FillStringModifiers(variable_definition, value, level, visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
    tag_scope->FillStringModifiers(variable_definition, value, level, visited);
  }
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetNumericValue(const StringId& variable)
    -> std::expected<Scope::NumericValue, ErrorCode> {
  if (const auto var_type = GetVariableDefinitions()->GetVariableType(variable);
      var_type != ruleset::VariableDefinitions<BaseTypes>::VariableType::kNumeric) {
    spdlog::warn("variable {} must be numeric, but it is {}", variable, var_type);
    return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
  }

  auto vardef = GetVariableDefinitions()->FindNumericVariable(variable);
  if (!vardef) {
    return std::unexpected(vardef.error());
  }

  NumericValue add{0};
  NumericValue mult{0};

  VisitedScopes visited;
  FillNumericModifiers(*vardef, add, mult, visited);

  mult = 1 + mult;
  mult = std::max<NumericValue>(mult, 0);

  auto value = add * mult;

  value = std::clamp(value, vardef->minimum, vardef->maximum);

  return value;
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetStringValue(const StringId& variable)
    -> std::expected<Scope::StringId, ErrorCode> {
  if (const auto var_type = GetVariableDefinitions()->GetVariableType(variable);
      var_type != ruleset::VariableDefinitions<BaseTypes>::VariableType::kString) {
    spdlog::warn("variable {} must be string, but it is {}", variable, var_type);
    return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
  }

  auto vardef = GetVariableDefinitions()->FindStringVariable(variable);
  if (!vardef) {
    return std::unexpected(vardef.error());
  }

  NumericValue level{0};
  StringId result;

  VisitedScopes visited;
  FillStringModifiers(*vardef, result, level, visited);

  return result;
}

template <typename BaseTypes>
bool Scope<BaseTypes>::IsStringVariable(const StringId& variable) const {
  return GetVariableDefinitions()->IsStringVariable(variable);
}

template <typename BaseTypes>
bool Scope<BaseTypes>::IsNumericVariable(const StringId& variable) const {
  return GetVariableDefinitions()->IsNumericVariable(variable);
}

template <typename BaseTypes>
auto Scope<BaseTypes>::FindParameterizedNumericValues(const StringId& query)
    -> std::expected<std::vector<ParameterizedNumericValue>, ErrorCode> {
  const auto instances = GetVariableDefinitions()->FindParameterizedNumericVariables(query);
  if (!instances) {
    return std::unexpected(instances.error());
  }

  std::vector<ParameterizedNumericValue> result;
  result.reserve(instances->size());
  for (const auto& instance : *instances) {
    auto value = GetNumericValue(instance.variable_id);
    if (!value) {
      return std::unexpected(value.error());
    }
    result.push_back(ParameterizedNumericValue{
        .normalized_id = instance.normalized_id,
        .variable_id = instance.variable_id,
        .parameters = instance.parameters,
        .value = *value,
    });
  }

  return result;
}

template <typename BaseTypes>
auto Scope<BaseTypes>::FindParameterizedStringValues(const StringId& query)
    -> std::expected<std::vector<ParameterizedStringValue>, ErrorCode> {
  const auto instances = GetVariableDefinitions()->FindParameterizedStringVariables(query);
  if (!instances) {
    return std::unexpected(instances.error());
  }

  std::vector<ParameterizedStringValue> result;
  result.reserve(instances->size());
  for (const auto& instance : *instances) {
    auto value = GetStringValue(instance.variable_id);
    if (!value) {
      return std::unexpected(value.error());
    }
    result.push_back(ParameterizedStringValue{
        .normalized_id = instance.normalized_id,
        .variable_id = instance.variable_id,
        .parameters = instance.parameters,
        .value = *value,
    });
  }

  return result;
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::SetNumericModifier(const StringId& variable,
                                                                    const StringId& key,
                                                                    NumericValue add,
                                                                    NumericValue mult,
                                                                    size_t modificationTime) {
  return numeric_variables_[variable].SetModifier(key, add, mult, modificationTime);
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::ChangeNumericModifier(const StringId& variable,
                                                                       const StringId& key,
                                                                       NumericValue add,
                                                                       NumericValue mult,
                                                                       size_t modificationTime) {
  return numeric_variables_[variable].ChangeModifier(key, add, mult, modificationTime);
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::SetStringModifier(const StringId& variable,
                                                                   const StringId& key,
                                                                   const StringId& value,
                                                                   NumericValue level,
                                                                   size_t modificationTime) {
  return string_variables_[variable].SetModifier(key, value, level, modificationTime);
}

template <typename BaseTypes>
std::expected<size_t, ErrorCode> Scope<BaseTypes>::GetModificationTime(
    const StringId& variable) const {
  auto vardef = GetVariableDefinitions()->FindVariable(variable);
  if (!vardef) {
    return std::unexpected(vardef.error());
  }

  VisitedScopes visited;
  return DoGetModificationTime(*vardef, visited);
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
