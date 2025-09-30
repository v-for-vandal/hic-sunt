#pragma once

namespace hs::scope {

template <typename BaseTypes>
void Scope<BaseTypes>::FillNumericModifiers(const StringId &variable,
    NumericValue& add, NumericValue& mult) const
{
  auto it = numeric_variables_.find(variable);
  if (it != numeric_variables_.end()) {
      it->second.FillNumericModifiers(add, mult);
  }

  if(parent_ != nullptr) {
      parent_->FillNumericModifiers(variable, add, mult);
  }


}

template <typename BaseTypes>
double Scope<BaseTypes>::GetNumericValue(const StringId &variable)
{
  const NumericVariableDefinition vardef = definitions_->FindNumericVariable(
      variable);

  NumericValue add{0};
  NumericValue mult{0};

  FillNumericModifiers(variable, add, mult);


  mult = 1 + mult;
  mult = max(mult, 0);

  auto value = add * mult;

  value = std::clamp(value, vardef->minimum, vardef->maximum);

  return value;
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
