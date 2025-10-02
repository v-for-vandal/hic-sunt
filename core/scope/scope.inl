#pragma once

#include <core/utils/serialize.hpp>

namespace hs::scope {

template <typename BaseTypes>
Scope<BaseTypes>::Scope(
  StringId id):
    id_(id)
{
}

template <typename BaseTypes>
void Scope<BaseTypes>::FillNumericModifiers(const StringId &variable,
    NumericValue& add, NumericValue& mult) const
{
  auto it = numeric_variables_.find(variable);
  if (it != numeric_variables_.end()) {
      it->second.FillModifiers(add, mult);
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
        fit->second.FillModifiers(value, level);
    }

    // get parent value
    if(parent_ != nullptr) {
        parent_->FillStringModifiers(variable, value, level);
    }

}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetNumericValue(const StringId &variable) -> Scope::NumericValue
{
    /*
  const NumericVariableDefinition vardef = definitions_->FindNumericVariable(
      variable);
      */

  NumericValue add{0};
  NumericValue mult{0};

  FillNumericModifiers(variable, add, mult);


  mult = 1 + mult;
  mult = std::max<NumericValue>(mult, 0);

  auto value = add * mult;

  //value = std::clamp(value, vardef->minimum, vardef->maximum);

  return value;
}

template <typename BaseTypes>
auto Scope<BaseTypes>::GetStringValue(const StringId &variable) -> Scope::StringId {
    NumericValue level{0};
    StringId result;

    FillStringModifiers(variable, result, level);

    return result;
}

template <typename BaseTypes>
bool Scope<BaseTypes>::AddNumericModifier(const StringId &variable, const StringId &key,
                       NumericValue add, NumericValue mult)
{
    numeric_variables_[variable].AddModifiers(key, add, mult);
    return true;
}

template <typename BaseTypes>
bool Scope<BaseTypes>::AddStringModifier(const StringId &variable, const StringId &key,
                       const StringId& value, NumericValue level)
{
    string_variables_[variable].AddModifiers(key, value, level);
    return true;
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
