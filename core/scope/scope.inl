#pragma once

#include "core/ruleset/variable_definition.hpp"
#include "core/types/error_code.hpp"
#include "scope.hpp"
#include "spdlog/spdlog.h"

#include <core/utils/serialize.hpp>

namespace hs::scope {

template <typename BaseTypes>
Scope<BaseTypes>::Scope(
  StringId id,
  types::ScopeType scope_type):
    id_(id),
    scope_type_(scope_type)
{
  if( BaseTypes::IsNullToken(id_)) {
    const uint64_t address_as_uint = reinterpret_cast<uint64_t>(this);
    id_ = BaseTypes::StringIdFromStdString(
      fmt::format("{:x}", address_as_uint));
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::SetVariableDefinitions(const VariableDefinitionsConstPtr& definitions)
{
    if (scope_type_ != types::ScopeType::SCOPE_TYPE_WORLD) {
        spdlog::warn("Setting definitiosn on non-root scope is not recommended");
    }
   cached_definitions_ = definitions;
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillNumericModifiers(const StringId &variable,
    NumericValue& add, NumericValue& mult) const
{
  VisitedScopes visited;
  FillNumericModifiers(variable, add, mult, visited);
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillNumericModifiers(const StringId &variable,
    NumericValue& add, NumericValue& mult, VisitedScopes& visited) const
{
  if (!visited.insert(this).second) {
    return;
  }

  auto it = numeric_variables_.find(variable);
  if (it != numeric_variables_.end()) {
      it->second.CalculateModifiers(add, mult);
  }

  if(parent_ != nullptr) {
      parent_->FillNumericModifiers(variable, add, mult, visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
      if (tag_scope != nullptr) {
          tag_scope->FillNumericModifiers(variable, add, mult, visited);
      }
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillStringModifiers(const StringId& variable,
    StringId& value, NumericValue& level)
{
    VisitedScopes visited;
    FillStringModifiers(variable, value, level, visited);
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillStringModifiers(const StringId& variable,
    StringId& value, NumericValue& level, VisitedScopes& visited)
{
    if (!visited.insert(this).second) {
        return;
    }

    auto fit = string_variables_.find(variable);
    if( fit != string_variables_.end()) {
        fit->second.CalculateModifiers(value, level);
    }

    // get parent value
    if(parent_ != nullptr) {
        parent_->FillStringModifiers(variable, value, level, visited);
    }

    for (const auto& tag_scope : tag_scopes_) {
        tag_scope->FillStringModifiers(variable, value, level, visited);
    }

}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetNumericValue(const StringId &variable) -> std::expected<Scope::NumericValue, ErrorCode>
{

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

  FillNumericModifiers(variable, add, mult);

  mult = 1 + mult;
  mult = std::max<NumericValue>(mult, 0);

  auto value = add * mult;

  value = std::clamp(value, vardef->minimum, vardef->maximum);

  return value;
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetStringValue(const StringId &variable) -> std::expected<Scope::StringId, ErrorCode> {
    if (!IsStringVariable(variable)) {
        spdlog::error("Variable {} is not of type string", variable);
        return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
    }

    NumericValue level{0};
    StringId result;

    FillStringModifiers(variable, result, level);

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
std::expected<void, ErrorCode> Scope<BaseTypes>::SetNumericModifier(const StringId &variable, const StringId &key,
                       NumericValue add, NumericValue mult, size_t modificationTime)
{
    return numeric_variables_[variable].SetModifier(key, add, mult, modificationTime);
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::ChangeNumericModifier(const StringId &variable, const StringId &key,
                       NumericValue add, NumericValue mult, size_t modificationTime)
{
    return numeric_variables_[variable].ChangeModifier(key, add, mult, modificationTime);
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::SetStringModifier(const StringId &variable, const StringId &key,
                       const StringId& value, NumericValue level, size_t modificationTime)
{
    return string_variables_[variable].SetModifier(key, value, level, modificationTime);
}



template <typename BaseTypes>
std::expected<size_t, ErrorCode> Scope<BaseTypes>::GetModificationTime(const StringId& variable) const  {
   VisitedScopes visited;
   return DoGetModificationTime(variable, visited);
}

template <typename BaseTypes>
std::expected<size_t, ErrorCode> Scope<BaseTypes>::DoGetModificationTime(const StringId& variable,
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
       auto parent_modification_time = parent_->DoGetModificationTime(variable, visited);
       if (!parent_modification_time.has_value()) {
           return parent_modification_time;
       }
       modification_time = std::max(modification_time, *parent_modification_time);
   }

   for (const auto& tag_scope : tag_scopes_) {
       auto tag_modification_time = tag_scope->DoGetModificationTime(variable, visited);
       if (!tag_modification_time.has_value()) {
           return tag_modification_time;
       }
       modification_time = std::max(modification_time, *tag_modification_time);
   }

   if (modification_time != 0) {
       return modification_time;
   }

   const auto& definitions = GetVariableDefinitions();
   if (!definitions->IsVariable(variable)) {
       return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
   }

   return 0;
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetVariableDefinitions() const ->const VariableDefinitionsConstPtr&
{

    if (cached_definitions_->IsEmpty()) {
        if (parent_) {
            cached_definitions_ = parent_->GetVariableDefinitions();
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
      it->second.ExplainModifiers([this, &collect_fn, &variable](const StringId& modifier,
              NumericValue add, NumericValue mult) {
          collect_fn(this->id_, variable, modifier, add, mult);
          });

  }

  if(parent_ != nullptr) {
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
              const StringId& value, NumericValue level) {
          collect_fn(this->id_, variable, modifier, value, level);
          });

  }

  if(parent_ != nullptr) {
      parent_->DoExplainStringVariable(variable, std::forward<CollectFn>(collect_fn), visited);
  }

  for (const auto& tag_scope : tag_scopes_) {
        tag_scope->DoExplainStringVariable(variable, std::forward<CollectFn>(collect_fn), visited);
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::ExplainAllVariables(auto&& collect_fn) {
    // TODO: Replace empty id with hex-string for address
    for(const auto& [k,v] : numeric_variables_) {
        v.ExplainModifiers([this, &collect_fn, &k] (const StringId& modifier,
              NumericValue add, NumericValue mult) {
              collect_fn(this->id_, k, modifier, add, mult);
          });
    }
    for(const auto& [k,v] : string_variables_) {
        v.ExplainModifiers([this, &collect_fn, &k] (const StringId& modifier,
              const StringId& value, NumericValue level) {
              collect_fn(this->id_, k, modifier, value, level);
          });
    }
  if(parent_ != nullptr) {
      parent_->ExplainAllVariables(collect_fn);
  }

}

template <typename BaseTypes>
std::expected<void, ErrorCode> Scope<BaseTypes>::AddTagLink([[maybe_unused]] const StringId&  tag_name,
    const ScopePtr& tag_scope) {

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
void SerializeTo(const Scope<BaseTypes> &source,
                 proto::scope::Scope &target) {
  target.Clear();
  target.set_id(BaseTypes::ToProtoString(source.id_));
  target.set_scope_type(source.scope_type_);
  // TODO: finish other serialization
}

template <typename BaseTypes>
Scope<BaseTypes> ParseFrom(const proto::scope::Scope &scope,
                            serialize::To<Scope<BaseTypes>>) {
  Scope<BaseTypes> result;
  result.id_ = ParseFrom(scope.id(), serialize::To<typename BaseTypes::StringId>{});
  result.scope_type_ = scope.scope_type();

  // TODO: Finish other serializations

  return result;
}

} // namespace hs::types
