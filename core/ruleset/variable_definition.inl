#pragma once

#include <type_traits>

#include "variable_definition.hpp"

namespace hs::ruleset {

template <typename BaseTypes>
typename VariableDefinitions<BaseTypes>::ParsedVariableDefinition
VariableDefinitions<BaseTypes>::ParseFromProto(const proto::ruleset::Variable& definition) {
  const auto variable_id = BaseTypes::StringIdFromStdString(definition.id());
  const bool is_parameterized = definition.id().find('{') != std::string::npos;

  const auto apply_allowed_scopes = [&definition](auto& target) {
    if (definition.has_allowed_scopes()) {
      target.allowed_scopes = types::ToScopeTypeFilter(definition.allowed_scopes());
    }
  };

  const auto finalize_numeric =
      [&](NumericVariableDefinition<BaseTypes> numeric_definition) -> ParsedVariableDefinition {
    numeric_definition.id = variable_id;
    if (!is_parameterized) {
      return numeric_definition;
    }

    auto parsed = details::ParseParameterizedDefinition<BaseTypes>(variable_id,
                                                                   std::move(numeric_definition));
    if (!parsed) {
      return parsed.error();
    }
    return std::move(*parsed);
  };

  const auto finalize_string =
      [&](StringVariableDefinition<BaseTypes> string_definition) -> ParsedVariableDefinition {
    string_definition.id = variable_id;
    if (!is_parameterized) {
      return string_definition;
    }

    auto parsed =
        details::ParseParameterizedDefinition<BaseTypes>(variable_id, std::move(string_definition));
    if (!parsed) {
      return parsed.error();
    }
    return std::move(*parsed);
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
    return finalize_numeric(std::move(numeric_definition));
  }

  if (definition.has_string()) {
    StringVariableDefinition<BaseTypes> string_definition;
    const auto& string_ = definition.string();
    if (!string_.default_().empty()) {
      string_definition.default_value = BaseTypes::StringIdFromStdString(string_.default_());
    }
    apply_allowed_scopes(string_definition);
    return finalize_string(std::move(string_definition));
  }

  if (definition.has_boolean()) {
    NumericVariableDefinition<BaseTypes> numeric_definition;
    numeric_definition.minimum = 0;
    numeric_definition.maximum = 1;
    apply_allowed_scopes(numeric_definition);
    return finalize_numeric(std::move(numeric_definition));
  }

  return ErrorCode::ERR_INCORRECT_VARIABLE_TYPE;
}

template <typename BaseTypes>
std::expected<typename VariableDefinitions<BaseTypes>::StringId, ErrorCode>
VariableDefinitions<BaseTypes>::AddNumericDefinition(
    const StringId& id, NumericVariableDefinition<BaseTypes> definition) {
  if (const auto var_type = GetVariableType(id);
      var_type != VariableType::kNumeric && var_type != VariableType::kMissing) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
  }

  definition.id = id;
  numeric_definitions_[id] = std::move(definition);
  return id;
}

template <typename BaseTypes>
std::expected<typename VariableDefinitions<BaseTypes>::StringId, ErrorCode>
VariableDefinitions<BaseTypes>::AddStringDefinition(
    const StringId& id, StringVariableDefinition<BaseTypes> definition) {
  if (const auto var_type = GetVariableType(id);
      var_type != VariableType::kString && var_type != VariableType::kMissing) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
  }

  definition.id = id;
  string_definitions_[id] = std::move(definition);
  return id;
}

template <typename BaseTypes>
std::expected<typename VariableDefinitions<BaseTypes>::StringId, ErrorCode>
VariableDefinitions<BaseTypes>::AddParameterizedNumericDefinition(
    const StringId& pattern, NumericVariableDefinition<BaseTypes> definition) {
  return AddParameterizedDefinition(pattern, std::move(definition),
                                    parameterized_numeric_definitions_);
}

template <typename BaseTypes>
std::expected<typename VariableDefinitions<BaseTypes>::StringId, ErrorCode>
VariableDefinitions<BaseTypes>::AddParameterizedStringDefinition(
    const StringId& pattern, StringVariableDefinition<BaseTypes> definition) {
  return AddParameterizedDefinition(pattern, std::move(definition),
                                    parameterized_string_definitions_);
}

template <typename BaseTypes>
std::expected<typename VariableDefinitions<BaseTypes>::StringId, ErrorCode>
VariableDefinitions<BaseTypes>::AddParameterizedNumericDefinition(
    ParameterizedNumericDefinition definition) {
  return AddParameterizedDefinition(std::move(definition), parameterized_numeric_definitions_);
}

template <typename BaseTypes>
std::expected<typename VariableDefinitions<BaseTypes>::StringId, ErrorCode>
VariableDefinitions<BaseTypes>::AddParameterizedStringDefinition(
    ParameterizedStringDefinition definition) {
  return AddParameterizedDefinition(std::move(definition), parameterized_string_definitions_);
}

template <typename BaseTypes>
std::expected<void, ErrorCode> VariableDefinitions<BaseTypes>::SetFixedParameterDomainValues(
    FixedParameterDomain domain, std::vector<StringId> values) {
  auto domain_values = std::make_shared<FixedParameterDomainValues<BaseTypes>>();
  domain_values->ordered_values.reserve(values.size());
  domain_values->allowed_values.reserve(values.size());

  for (const auto& value : values) {
    const auto view_holder = details::MakeStringIdView<BaseTypes>(value);
    if (!details::IsValidConcreteParameterValue(details::GetView(view_holder))) {
      return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);
    }
    if (domain_values->allowed_values.insert(value).second) {
      domain_values->ordered_values.push_back(value);
    }
  }

  fixed_parameter_domain_values_[domain] = std::move(domain_values);

  for (auto& [_, definition] : parameterized_numeric_definitions_) {
    auto result = FillFixedParameterValues(definition);
    if (!result) {
      return result;
    }
  }
  for (auto& [_, definition] : parameterized_string_definitions_) {
    auto result = FillFixedParameterValues(definition);
    if (!result) {
      return result;
    }
  }

  return {};
}

template <typename BaseTypes>
template <typename ConcreteDefinition>
std::expected<void, ErrorCode> VariableDefinitions<BaseTypes>::FillFixedParameterValues(
    ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition) const {
  for (auto& parameter : definition.parameters) {
    if (parameter.kind == ParameterDomainKind::kOpen) {
      parameter.fixed_values.reset();
      continue;
    }

    const auto fit = fixed_parameter_domain_values_.find(parameter.fixed_domain);
    if (fit == fixed_parameter_domain_values_.end()) {
      parameter.fixed_values.reset();
      continue;
    }

    parameter.fixed_values = fit->second;
  }

  return {};
}

template <typename BaseTypes>
template <typename ConcreteDefinition>
std::expected<typename VariableDefinitions<BaseTypes>::StringId, ErrorCode>
VariableDefinitions<BaseTypes>::AddParameterizedDefinition(
    const StringId& pattern, ConcreteDefinition definition,
    ParameterizedDefinitionsMap<ConcreteDefinition>& target) {
  auto parsed = details::ParseParameterizedDefinition<BaseTypes>(pattern, std::move(definition));
  if (!parsed) {
    return std::unexpected(parsed.error());
  }

  return AddParameterizedDefinition(std::move(*parsed), target);
}

template <typename BaseTypes>
template <typename ConcreteDefinition>
std::expected<typename VariableDefinitions<BaseTypes>::StringId, ErrorCode>
VariableDefinitions<BaseTypes>::AddParameterizedDefinition(
    ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition> definition,
    ParameterizedDefinitionsMap<ConcreteDefinition>& target) {
  const bool is_numeric_definition =
      std::is_same_v<ConcreteDefinition, NumericVariableDefinition<BaseTypes>>;
  if constexpr (std::is_same_v<ConcreteDefinition, NumericVariableDefinition<BaseTypes>>) {
    if (parameterized_string_definitions_.contains(definition.id)) {
      return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
    }
  } else {
    if (parameterized_numeric_definitions_.contains(definition.id)) {
      return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
    }
  }

  if (const auto var_type = GetVariableType(definition.id); var_type != VariableType::kMissing) {
    const auto expected_type =
        is_numeric_definition ? VariableType::kNumeric : VariableType::kString;
    if (var_type != expected_type) {
      return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
    }
  }

  auto fill_result = FillFixedParameterValues(definition);
  if (!fill_result) {
    return std::unexpected(fill_result.error());
  }

  auto normalized_id = definition.id;
  target[normalized_id] = std::move(definition);
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

    const auto parameterized_fit = parameterized_numeric_definitions_.find(query.raw_id);
    if (parameterized_fit != parameterized_numeric_definitions_.end()) {
      return static_cast<const NumericVariableDefinition<BaseTypes>&>(parameterized_fit->second);
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

    const auto parameterized_fit = parameterized_string_definitions_.find(query.raw_id);
    if (parameterized_fit != parameterized_string_definitions_.end()) {
      return static_cast<const StringVariableDefinition<BaseTypes>&>(parameterized_fit->second);
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

  auto definition = static_cast<const ConcreteDefinition&>(*parameterized_definition);
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
        (parameter.fixed_values == nullptr ||
         !parameter.fixed_values->allowed_values.contains(argument.value))) {
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
