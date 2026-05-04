#pragma once

#include "scope_change_set.hpp"

namespace hs::scope {

template <typename BaseTypes>
std::expected<void, ErrorCode>
ScopeChangeSet<BaseTypes>::ValidateNumericVariable(const StringId& variable,
                                                   const StringId& key) const {
  if (BaseTypes::IsNullToken(key)) {
    return std::unexpected(ErrorCode::ERR_EMPTY_MODIFIER_KEY);
  }

  if (!target_scope_->IsNumericVariable(variable)) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
  }

  return {};
}

template <typename BaseTypes>
std::expected<void, ErrorCode>
ScopeChangeSet<BaseTypes>::ValidateStringVariable(const StringId& variable,
                                                  const StringId& key) const {
  if (BaseTypes::IsNullToken(key)) {
    return std::unexpected(ErrorCode::ERR_EMPTY_MODIFIER_KEY);
  }

  if (!target_scope_->IsStringVariable(variable)) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
  }

  return {};
}

template <typename BaseTypes>
std::expected<void, ErrorCode> ScopeChangeSet<BaseTypes>::SetNumericModifier(
    const StringId& variable, const StringId& key, NumericValue add,
    NumericValue mult) {
  auto validation_result = ValidateNumericVariable(variable, key);
  if (!validation_result) {
    return validation_result;
  }

  operations_.push_back(Operation{
      .kind = OperationKind::kSetNumericModifier,
      .variable = variable,
      .key = key,
      .add = add,
      .mult = mult,
  });

  return {};
}

template <typename BaseTypes>
std::expected<void, ErrorCode> ScopeChangeSet<BaseTypes>::ChangeNumericModifier(
    const StringId& variable, const StringId& key, NumericValue add,
    NumericValue mult) {
  auto validation_result = ValidateNumericVariable(variable, key);
  if (!validation_result) {
    return validation_result;
  }

  operations_.push_back(Operation{
      .kind = OperationKind::kChangeNumericModifier,
      .variable = variable,
      .key = key,
      .add = add,
      .mult = mult,
  });

  return {};
}

template <typename BaseTypes>
std::expected<void, ErrorCode> ScopeChangeSet<BaseTypes>::SetStringModifier(
    const StringId& variable, const StringId& key, const StringId& value,
    NumericValue level) {
  auto validation_result = ValidateStringVariable(variable, key);
  if (!validation_result) {
    return validation_result;
  }

  operations_.push_back(Operation{
      .kind = OperationKind::kSetStringModifier,
      .variable = variable,
      .key = key,
      .value = value,
      .level = level,
  });

  return {};
}

template <typename BaseTypes>
std::expected<void, ErrorCode> ScopeChangeSet<BaseTypes>::Apply(
    size_t modification_time) const {
  for (const auto& operation : operations_) {
    std::expected<void, ErrorCode> result;

    switch (operation.kind) {
      case OperationKind::kSetNumericModifier:
        result = target_scope_->SetNumericModifier(operation.variable,
                                                  operation.key,
                                                  operation.add,
                                                  operation.mult,
                                                  modification_time);
        break;
      case OperationKind::kChangeNumericModifier:
        result = target_scope_->ChangeNumericModifier(operation.variable,
                                                     operation.key,
                                                     operation.add,
                                                     operation.mult,
                                                     modification_time);
        break;
      case OperationKind::kSetStringModifier:
        result = target_scope_->SetStringModifier(operation.variable,
                                                 operation.key,
                                                 operation.value,
                                                 operation.level,
                                                 modification_time);
        break;
    }

    if (!result) {
      return result;
    }
  }

  return {};
}

}  // namespace hs::scope
