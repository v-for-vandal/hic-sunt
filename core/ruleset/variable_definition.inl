#pragma once

#include <type_traits>

#include "variable_definition.hpp"

namespace hs::ruleset {

template <typename BaseTypes>
typename VariableDefinitions<BaseTypes>::ParsedVariableDefinition
VariableDefinitions<BaseTypes>::ParseFromProto(const proto::ruleset::Variable& definition) {
  const auto apply_allowed_scopes = [&definition](auto& target) {
    if (definition.has_allowed_scopes()) {
      target.allowed_scopes = types::ToScopeTypeFilter(definition.allowed_scopes());
    }
  };

  if (definition.has_numeric()) {
    NumericVariableDefinition<BaseTypes> numeric_definition;
    const auto& numeric = definition.numeric();
    if (numeric.has_minimum()) {
      numeric_definition.minimum = numeric.minimum();
    }
    if (numeric.has_maximum()) {
      numeric_definition.maximum = numeric.maximum();
    }
    apply_allowed_scopes(numeric_definition);
    return numeric_definition;
  }

  if (definition.has_string()) {
    StringVariableDefinition<BaseTypes> string_definition;
    const auto& string_ = definition.string();
    if (!string_.default_().empty()) {
      [[maybe_unused]] const auto default_result = string_definition.default_variable.SetModifier(
          BaseTypes::StringIdFromStdString("default"),
          BaseTypes::StringIdFromStdString(string_.default_()), 0, 0);
    }
    apply_allowed_scopes(string_definition);
    return string_definition;
  }

  if (definition.has_boolean()) {
    NumericVariableDefinition<BaseTypes> numeric_definition;
    numeric_definition.minimum = 0;
    numeric_definition.maximum = 1;
    apply_allowed_scopes(numeric_definition);
    return numeric_definition;
  }

  return ErrorCode::ERR_INCORRECT_VARIABLE_TYPE;
}

template <typename BaseTypes>
std::expected<void, ErrorCode> VariableDefinitions<BaseTypes>::AddNumericDefinition(
    const StringId& id, NumericVariableDefinition<BaseTypes> definition) {
  if (const auto var_type = GetVariableType(id);
      var_type != VariableType::kNumeric && var_type != VariableType::kMissing) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
  }

  definition.id = id;
  numeric_definitions_[id] = std::move(definition);
  return {};
}

template <typename BaseTypes>
std::expected<void, ErrorCode> VariableDefinitions<BaseTypes>::AddStringDefinition(
    const StringId& id, StringVariableDefinition<BaseTypes> definition) {
  if (const auto var_type = GetVariableType(id);
      var_type != VariableType::kString && var_type != VariableType::kMissing) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
  }

  definition.id = id;
  string_definitions_[id] = std::move(definition);
  return {};
}

template <typename BaseTypes>
std::expected<typename VariableDefinitions<BaseTypes>::StringId, ErrorCode>
VariableDefinitions<BaseTypes>::AddParameterizedNumericDefinition(
    const StringId& pattern, std::vector<ParameterDomain<BaseTypes>> parameters,
    NumericVariableDefinition<BaseTypes> definition) {
  return AddParameterizedDefinition(pattern, std::move(parameters), std::move(definition),
                                    parameterized_numeric_definitions_);
}

template <typename BaseTypes>
std::expected<typename VariableDefinitions<BaseTypes>::StringId, ErrorCode>
VariableDefinitions<BaseTypes>::AddParameterizedStringDefinition(
    const StringId& pattern, std::vector<ParameterDomain<BaseTypes>> parameters,
    StringVariableDefinition<BaseTypes> definition) {
  return AddParameterizedDefinition(pattern, std::move(parameters), std::move(definition),
                                    parameterized_string_definitions_);
}

template <typename BaseTypes>
template <typename ConcreteDefinition>
std::expected<typename VariableDefinitions<BaseTypes>::StringId, ErrorCode>
VariableDefinitions<BaseTypes>::AddParameterizedDefinition(
    const StringId& pattern, std::vector<ParameterDomain<BaseTypes>> parameters,
    ConcreteDefinition definition, ParameterizedDefinitionsMap<ConcreteDefinition>& target) {
  auto parsed = details::ParseParameterizedDefinition<BaseTypes>(pattern, std::move(parameters),
                                                                 std::move(definition));
  if (!parsed) {
    return std::unexpected(parsed.error());
  }

  const bool is_numeric_definition =
      std::is_same_v<ConcreteDefinition, NumericVariableDefinition<BaseTypes>>;
  if constexpr (std::is_same_v<ConcreteDefinition, NumericVariableDefinition<BaseTypes>>) {
    if (parameterized_string_definitions_.contains(parsed->id)) {
      return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
    }
  } else {
    if (parameterized_numeric_definitions_.contains(parsed->id)) {
      return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
    }
  }

  if (const auto var_type = GetVariableType(parsed->id); var_type != VariableType::kMissing) {
    const auto expected_type =
        is_numeric_definition ? VariableType::kNumeric : VariableType::kString;
    if (var_type != expected_type) {
      return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
    }
  }

  auto normalized_id = parsed->id;
  target[normalized_id] = std::move(*parsed);
  return normalized_id;
}

template <typename BaseTypes>
typename VariableDefinitions<BaseTypes>::VariableType
VariableDefinitions<BaseTypes>::GetVariableType(const StringId& id) const noexcept {
  if (numeric_definitions_.contains(id)) {
    return VariableType::kNumeric;
  }
  if (string_definitions_.contains(id)) {
    return VariableType::kString;
  }
  if (parameterized_numeric_definitions_.contains(id)) {
    return VariableType::kNumeric;
  }
  if (parameterized_string_definitions_.contains(id)) {
    return VariableType::kString;
  }

  const auto parsed = ParseVariableQuery<BaseTypes>(id, false);
  if (!parsed || !parsed->is_parameterized || parsed->has_wildcard) {
    return VariableType::kMissing;
  }
  if (parameterized_numeric_definitions_.contains(parsed->normalized_id)) {
    return VariableType::kNumeric;
  }
  if (parameterized_string_definitions_.contains(parsed->normalized_id)) {
    return VariableType::kString;
  }
  return VariableType::kMissing;
}

template <typename BaseTypes>
typename VariableDefinitions<BaseTypes>::VariableType
VariableDefinitions<BaseTypes>::GetVariableType(
    const ParsedVariableQuery<BaseTypes>& query) const noexcept {
  if (!query.is_parameterized) {
    if (numeric_definitions_.contains(query.raw_id)) {
      return VariableType::kNumeric;
    }
    if (string_definitions_.contains(query.raw_id)) {
      return VariableType::kString;
    }
    return VariableType::kMissing;
  }

  if (query.has_wildcard) {
    return VariableType::kMissing;
  }
  if (parameterized_numeric_definitions_.contains(query.normalized_id)) {
    return VariableType::kNumeric;
  }
  if (parameterized_string_definitions_.contains(query.normalized_id)) {
    return VariableType::kString;
  }
  return VariableType::kMissing;
}

template <typename BaseTypes>
std::expected<NumericVariableDefinition<BaseTypes>, ErrorCode>
VariableDefinitions<BaseTypes>::FindNumericVariable(const StringId& id) const {
  auto parsed = ParseVariableQuery<BaseTypes>(id, false);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  return FindNumericVariable(*parsed);
}

template <typename BaseTypes>
std::expected<NumericVariableDefinition<BaseTypes>, ErrorCode>
VariableDefinitions<BaseTypes>::FindNumericVariable(
    const ParsedVariableQuery<BaseTypes>& query) const {
  if (!query.is_parameterized) {
    const auto fit = numeric_definitions_.find(query.raw_id);
    if (fit != numeric_definitions_.end()) {
      return fit->second;
    }

    spdlog::error("Variable {} is unknown or is not numeric", query.raw_id);
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  auto parameterized = FindParameterizedVariable(query, parameterized_numeric_definitions_);
  if (parameterized) {
    return *parameterized;
  }
  if (parameterized.error() != ErrorCode::ERR_NO_SUCH_VARIABLE) {
    return std::unexpected(parameterized.error());
  }

  spdlog::error("Variable {} is unknown or is not numeric", query.raw_id);
  return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
}

template <typename BaseTypes>
std::expected<StringVariableDefinition<BaseTypes>, ErrorCode>
VariableDefinitions<BaseTypes>::FindStringVariable(const StringId& id) const {
  auto parsed = ParseVariableQuery<BaseTypes>(id, false);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  return FindStringVariable(*parsed);
}

template <typename BaseTypes>
std::expected<StringVariableDefinition<BaseTypes>, ErrorCode>
VariableDefinitions<BaseTypes>::FindStringVariable(
    const ParsedVariableQuery<BaseTypes>& query) const {
  if (!query.is_parameterized) {
    const auto fit = string_definitions_.find(query.raw_id);
    if (fit != string_definitions_.end()) {
      return fit->second;
    }

    spdlog::error("Variable {} is unknown or is not string", query.raw_id);
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  auto parameterized = FindParameterizedVariable(query, parameterized_string_definitions_);
  if (parameterized) {
    return *parameterized;
  }
  if (parameterized.error() != ErrorCode::ERR_NO_SUCH_VARIABLE) {
    return std::unexpected(parameterized.error());
  }

  spdlog::error("Variable {} is unknown or is not string", query.raw_id);
  return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
}

template <typename BaseTypes>
template <typename ConcreteDefinition>
std::expected<ConcreteDefinition, ErrorCode>
VariableDefinitions<BaseTypes>::FindParameterizedVariable(
    const StringId& id, const ParameterizedDefinitionsMap<ConcreteDefinition>& definitions) const {
  auto parsed = ParseVariableQuery<BaseTypes>(id, false);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  return FindParameterizedVariable(*parsed, definitions);
}

template <typename BaseTypes>
template <typename ConcreteDefinition>
std::expected<ConcreteDefinition, ErrorCode>
VariableDefinitions<BaseTypes>::FindParameterizedVariable(
    const ParsedVariableQuery<BaseTypes>& query,
    const ParameterizedDefinitionsMap<ConcreteDefinition>& definitions) const {
  if (query.has_wildcard) {
    return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
  }

  auto parameterized_definition = FindParameterizedDefinition(query, definitions);
  if (!parameterized_definition) {
    return std::unexpected(parameterized_definition.error());
  }

  auto definition = parameterized_definition->concrete_definition;
  const auto concrete_id = details::BuildConcreteVariableId(*parameterized_definition, query);
  if (!concrete_id) {
    return std::unexpected(concrete_id.error());
  }
  definition.id = *concrete_id;
  return definition;
}

template <typename BaseTypes>
template <typename ConcreteDefinition>
std::expected<ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>, ErrorCode>
VariableDefinitions<BaseTypes>::FindParameterizedDefinition(
    const ParsedVariableQuery<BaseTypes>& query,
    const ParameterizedDefinitionsMap<ConcreteDefinition>& definitions) const {
  if (!query.is_parameterized) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  const auto fit = definitions.find(query.normalized_id);
  if (fit == definitions.end()) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }
  if (query.arguments.size() != fit->second.parameters.size()) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  for (size_t idx = 0; idx < query.arguments.size(); ++idx) {
    const auto& argument = query.arguments[idx];
    const auto& parameter = fit->second.parameters[idx];
    if (argument.kind == ParsedVariableQueryArgumentKind::kConcrete &&
        parameter.kind == ParameterDomainKind::kFixed &&
        !parameter.allowed_values.contains(argument.value)) {
      return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
    }
  }

  return fit->second;
}

template <typename BaseTypes>
std::expected<typename VariableDefinitions<BaseTypes>::ParameterizedNumericDefinition, ErrorCode>
VariableDefinitions<BaseTypes>::FindParameterizedNumericDefinition(
    const ParsedVariableQuery<BaseTypes>& query) const {
  return FindParameterizedDefinition(query, parameterized_numeric_definitions_);
}

template <typename BaseTypes>
std::expected<typename VariableDefinitions<BaseTypes>::ParameterizedStringDefinition, ErrorCode>
VariableDefinitions<BaseTypes>::FindParameterizedStringDefinition(
    const ParsedVariableQuery<BaseTypes>& query) const {
  return FindParameterizedDefinition(query, parameterized_string_definitions_);
}

template <typename BaseTypes>
std::expected<VariableDefinitionBase<BaseTypes>, ErrorCode>
VariableDefinitions<BaseTypes>::FindVariable(const StringId& id) const {
  auto parsed = ParseVariableQuery<BaseTypes>(id, false);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  return FindVariable(*parsed);
}

template <typename BaseTypes>
std::expected<VariableDefinitionBase<BaseTypes>, ErrorCode>
VariableDefinitions<BaseTypes>::FindVariable(const ParsedVariableQuery<BaseTypes>& query) const {
  const auto numeric = FindNumericVariable(query);
  if (numeric) {
    return numeric;
  }
  if (numeric.error() != ErrorCode::ERR_NO_SUCH_VARIABLE) {
    return std::unexpected(numeric.error());
  }

  const auto string_ = FindStringVariable(query);
  if (string_) {
    return string_;
  }
  if (string_.error() != ErrorCode::ERR_NO_SUCH_VARIABLE) {
    return std::unexpected(string_.error());
  }

  spdlog::error("Variable {} is unknown", query.raw_id);
  return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
}

template <typename BaseTypes>
std::expected<std::vector<typename VariableDefinitions<BaseTypes>::ParameterizedInstance>,
              ErrorCode>
VariableDefinitions<BaseTypes>::FindParameterizedNumericVariables(const StringId& query) const {
  return FindParameterizedVariables(query, parameterized_numeric_definitions_);
}

template <typename BaseTypes>
std::expected<std::vector<typename VariableDefinitions<BaseTypes>::ParameterizedInstance>,
              ErrorCode>
VariableDefinitions<BaseTypes>::FindParameterizedStringVariables(const StringId& query) const {
  return FindParameterizedVariables(query, parameterized_string_definitions_);
}

template <typename BaseTypes>
template <typename ConcreteDefinition>
auto VariableDefinitions<BaseTypes>::FindParameterizedVariables(
    const StringId& query, const ParameterizedDefinitionsMap<ConcreteDefinition>& definitions) const
    -> std::expected<std::vector<ParameterizedInstance>, ErrorCode> {
  const auto parsed = ParseVariableQuery<BaseTypes>(query, true);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  if (!parsed->is_parameterized) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  const auto fit = definitions.find(parsed->normalized_id);
  if (fit == definitions.end()) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  return details::GenerateParameterizedInstances(*parsed, fit->second);
}

}  // namespace hs::ruleset
