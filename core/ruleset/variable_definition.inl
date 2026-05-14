namespace hs::ruleset {

template <typename BaseTypes>
typename VariableDefinitions<BaseTypes>::ParsedVariableDefinition
VariableDefinitions<BaseTypes>::ParseFromProto(const proto::ruleset::Variable &definition) {
  const auto apply_allowed_scopes = [&definition](auto &target) {
    if (definition.has_allowed_scopes()) {
      target.allowed_scopes = types::ToScopeTypeFilter(definition.allowed_scopes());
    }
  };

  if (definition.has_numeric()) {
    NumericVariableDefinition<BaseTypes> numeric_definition;
    const auto &numeric = definition.numeric();
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
    const auto &string_ = definition.string();
    if (!string_.default_().empty()) {
      string_definition.default_value = BaseTypes::StringIdFromStdString(string_.default_());
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
    const StringId &id, NumericVariableDefinition<BaseTypes> definition) {
  if (const auto var_type = GetVariableType(id);
      var_type != VariableType::kNumeric && var_type != VariableType::kMissing) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
  }

  numeric_definitions_[id] = std::move(definition);
  return {};
}

template <typename BaseTypes>
std::expected<void, ErrorCode> VariableDefinitions<BaseTypes>::AddStringDefinition(
    const StringId &id, StringVariableDefinition<BaseTypes> definition) {
  if (const auto var_type = GetVariableType(id);
      var_type != VariableType::kString && var_type != VariableType::kMissing) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
  }

  string_definitions_[id] = std::move(definition);
  return {};
}

template <typename BaseTypes>
typename VariableDefinitions<BaseTypes>::VariableType VariableDefinitions<BaseTypes>::GetVariableType(
    const StringId &id) const noexcept {
  if (numeric_definitions_.contains(id)) {
    return VariableType::kNumeric;
  }
  if (string_definitions_.contains(id)) {
    return VariableType::kString;
  }
  return VariableType::kMissing;
}

template <typename BaseTypes>
std::expected<NumericVariableDefinition<BaseTypes>, ErrorCode>
VariableDefinitions<BaseTypes>::FindNumericVariable(const StringId &id) const {
  const auto fit = numeric_definitions_.find(id);
  if (fit == numeric_definitions_.end()) {
    spdlog::error("Variable {} is unknown or is not numeric", id);
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  return fit->second;
}

template <typename BaseTypes>
std::expected<StringVariableDefinition<BaseTypes>, ErrorCode>
VariableDefinitions<BaseTypes>::FindStringVariable(const StringId &id) const {
  const auto fit = string_definitions_.find(id);
  if (fit == string_definitions_.end()) {
    spdlog::error("Variable {} is unknown or is not string", id);
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  return fit->second;
}

}  // namespace hs::ruleset
