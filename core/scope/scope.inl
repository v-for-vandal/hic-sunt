#pragma once

#include "core/types/error_code.hpp"
#include "scope.hpp"

#include <core/utils/serialize.hpp>
#include "numeric_variable.hpp"

namespace hs::scope {

template <typename BaseTypes>
Scope<BaseTypes>::Scope(
  StringId id):
    id_(id)
{
  if( BaseTypes::IsNullToken(id_)) {
    const uint64_t address_as_uint = reinterpret_cast<uint64_t>(this);
    id_ = BaseTypes::StringIdFromStdString(
      fmt::format("{:x}", address_as_uint));
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillNumericModifiers(const StringId &variable,
    NumericValue& add, NumericValue& mult) const
{
  auto it = numeric_variables_.find(variable);
  if (it != numeric_variables_.end()) {
      it->second.CalculateModifiers(add, mult);
  }

  if(parent_ != nullptr) {
      parent_->FillNumericModifiers(variable, add, mult);
  }
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillStringModifiers(const StringId& variable,
    StringId& value, NumericValue& level)
{
    auto fit = string_variables_.find(variable);
    if( fit != string_variables_.end()) {
        fit->second.CalculateModifiers(value, level);
    }

    // get parent value
    if(parent_ != nullptr) {
        parent_->FillStringModifiers(variable, value, level);
    }

}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetNumericValue(const StringId &variable) -> std::expected<Scope::NumericValue, ErrorCode>
{
    if (!IsNumericVariable(variable)) {
        spdlog::error("Variable {} is not of type numeric", variable);
        return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
    }

  const auto& vardef = GetVariableDefinitions()->FindNumericVariable(
      variable);

  NumericValue add{0};
  NumericValue mult{0};

  FillNumericModifiers(variable, add, mult);

  mult = 1 + mult;
  mult = std::max<NumericValue>(mult, 0);

  auto value = add * mult;

  value = std::clamp(value, vardef.minimum, vardef.maximum);

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
   if (auto fit = numeric_variables_.find(variable); fit != numeric_variables_.end()) {
       return fit->second.GetModificationTime();
   }

   if (auto fit = string_variables_.find(variable); fit != string_variables_.end()) {
       return fit->second.GetModificationTime();
   }

   if (parent_ != nullptr) {
       return parent_->GetModificationTime(variable);
   }

   // parent is null, we are top level
   const auto& definitions = GetVariableDefinitions();
   if (!definitions_.IsVariable(variable)) {
       return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
   }

   return 0;
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetVariableDefinitions() const ->const VariableDefinitionsPtr&
{
    if (definitions_->IsEmpty()) {
        if (parent_) {
            definitions_ = parent_->GetVariableDefinitions();
        }
    }

    return definitions_;
}

template <typename BaseTypes>
void Scope<BaseTypes>::ExplainNumericVariable(const StringId& variable, auto&& collect_fn) {
  auto it = numeric_variables_.find(variable);
  if (it != numeric_variables_.end()) {
      it->second.ExplainModifiers([this, &collect_fn, &variable](const StringId& modifier,
              NumericValue add, NumericValue mult) {
          collect_fn(this->id_, variable, modifier, add, mult);
          });

  }

  if(parent_ != nullptr) {
      parent_->ExplainNumericVariable(variable, collect_fn);
  }

}

template <typename BaseTypes>
void Scope<BaseTypes>::ExplainStringVariable(const StringId& variable, auto&& collect_fn) {
  auto it = string_variables_.find(variable);
  if (it != string_variables_.end()) {
      it->second.ExplainModifiers([this, &collect_fn, &variable](const StringId& modifier,
              const StringId& value, NumericValue level) {
          collect_fn(this->id_, variable, modifier, value, level);
          });

  }

  if(parent_ != nullptr) {
      parent_->ExplainStringVariable(variable, collect_fn);
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
void SerializeTo(const Scope<BaseTypes> &source,
                 proto::scope::Scope &target) {
  target.Clear();
  target.set_id(BaseTypes::ToProtoString(source.id_));
  // TODO: finish other serialization
}

template <typename BaseTypes>
Scope<BaseTypes> ParseFrom(const proto::scope::Scope &scope,
                            serialize::To<Scope<BaseTypes>>) {
  Scope<BaseTypes> result;
  result.id_ = ParseFrom(scope.id(), serialize::To<typename BaseTypes::StringId>{});

  // TODO: Finish other serializations

  return result;
}

} // namespace hs::types
