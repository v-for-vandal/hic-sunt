#pragma once

#include <absl/container/flat_hash_map.h>
#include <ruleset/variables.pb.h>
#include <spdlog/spdlog.h>

#include <core/types/error_code.hpp>
#include <core/types/scope_type.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/non_null_ptr.hpp>
#include <expected>
#include <limits>
#include <variant>
#include <vector>

#include "core/ruleset/parameterized_variable_definition.hpp"
#include "core/types/variable_type.hpp"

namespace hs::ruleset {

template <typename BaseTypes = StdBaseTypes>
class VariableDefinitionBase {
 public:
  using StringId = typename BaseTypes::StringId;

  StringId id{};
  types::ScopeTypeFilter allowed_scopes = [] {
    types::ScopeTypeFilter result;
    result.set();
    return result;
  }();
};

template <typename BaseTypes = StdBaseTypes>
class NumericVariableDefinition : public VariableDefinitionBase<BaseTypes> {
 public:
  using NumericValue = typename BaseTypes::NumericValue;

  NumericValue maximum = std::numeric_limits<NumericValue>::max();
  NumericValue minimum = std::numeric_limits<NumericValue>::lowest();
};

template <typename BaseTypes = StdBaseTypes>
class StringVariableDefinition : public VariableDefinitionBase<BaseTypes> {
 public:
  using StringId = typename BaseTypes::StringId;

  StringId default_value{};
};

template <typename BaseTypes = StdBaseTypes>
using ParameterizedNumericVariableDefinition =
    ParameterizedVariableDefinition<BaseTypes, NumericVariableDefinition<BaseTypes>>;

template <typename BaseTypes = StdBaseTypes>
using ParameterizedStringVariableDefinition =
    ParameterizedVariableDefinition<BaseTypes, StringVariableDefinition<BaseTypes>>;

template <typename BaseTypes = StdBaseTypes>
class VariableDefinitions {
 public:
  using StringId = typename BaseTypes::StringId;
  using VariableType = types::VariableType;
  using ParsedVariableDefinition = std::variant<NumericVariableDefinition<BaseTypes>,
                                                StringVariableDefinition<BaseTypes>, ErrorCode>;
  using ParameterizedNumericDefinition = ParameterizedNumericVariableDefinition<BaseTypes>;
  using ParameterizedStringDefinition = ParameterizedStringVariableDefinition<BaseTypes>;
  using ParameterizedInstance = ParameterizedVariableInstance<BaseTypes>;

  static ParsedVariableDefinition ParseFromProto(const proto::ruleset::Variable& definition);

  bool IsEmpty() const noexcept {
    return string_definitions_.empty() && numeric_definitions_.empty() &&
           parameterized_numeric_definitions_.empty() && parameterized_string_definitions_.empty();
  }

  void Clear() {
    numeric_definitions_.clear();
    string_definitions_.clear();
    parameterized_numeric_definitions_.clear();
    parameterized_string_definitions_.clear();
  }

  std::expected<void, ErrorCode> AddNumericDefinition(
      const StringId& id, NumericVariableDefinition<BaseTypes> definition);

  std::expected<void, ErrorCode> AddStringDefinition(
      const StringId& id, StringVariableDefinition<BaseTypes> definition);

  std::expected<StringId, ErrorCode> AddParameterizedNumericDefinition(
      const StringId& pattern, std::vector<ParameterDomain<BaseTypes>> parameters,
      NumericVariableDefinition<BaseTypes> definition);

  std::expected<StringId, ErrorCode> AddParameterizedStringDefinition(
      const StringId& pattern, std::vector<ParameterDomain<BaseTypes>> parameters,
      StringVariableDefinition<BaseTypes> definition);

  bool IsNumericVariable(const StringId& id) const noexcept {
    return GetVariableType(id) == VariableType::kNumeric;
  }

  bool IsStringVariable(const StringId& id) const noexcept {
    return GetVariableType(id) == VariableType::kString;
  }

  bool IsVariable(const StringId& id) const noexcept {
    return GetVariableType(id) != VariableType::kMissing;
  }

  VariableType GetVariableType(const StringId& id) const noexcept;

  std::expected<NumericVariableDefinition<BaseTypes>, ErrorCode> FindNumericVariable(
      const StringId& id) const;

  std::expected<StringVariableDefinition<BaseTypes>, ErrorCode> FindStringVariable(
      const StringId& id) const;

  std::expected<VariableDefinitionBase<BaseTypes>, ErrorCode> FindVariable(
      const StringId& id) const;

  std::expected<std::vector<ParameterizedInstance>, ErrorCode> FindParameterizedNumericVariables(
      const StringId& query) const;

  std::expected<std::vector<ParameterizedInstance>, ErrorCode> FindParameterizedStringVariables(
      const StringId& query) const;

  const auto& GetNumericDefinitions() const noexcept { return numeric_definitions_; }

  const auto& GetStringDefinitions() const noexcept { return string_definitions_; }

  const auto& GetParameterizedNumericDefinitions() const noexcept {
    return parameterized_numeric_definitions_;
  }

  const auto& GetParameterizedStringDefinitions() const noexcept {
    return parameterized_string_definitions_;
  }

 private:
  template <typename ConcreteDefinition>
  using ParameterizedDefinitionsMap =
      absl::flat_hash_map<StringId, ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>>;

  template <typename ConcreteDefinition>
  std::expected<StringId, ErrorCode> AddParameterizedDefinition(
      const StringId& pattern, std::vector<ParameterDomain<BaseTypes>> parameters,
      ConcreteDefinition definition, ParameterizedDefinitionsMap<ConcreteDefinition>& target);

  template <typename ConcreteDefinition>
  std::expected<ConcreteDefinition, ErrorCode> FindParameterizedVariable(
      const StringId& id, const ParameterizedDefinitionsMap<ConcreteDefinition>& definitions) const;

  template <typename ConcreteDefinition>
  auto FindParameterizedVariables(
      const StringId& query, const ParameterizedDefinitionsMap<ConcreteDefinition>& definitions)
      const -> std::expected<std::vector<ParameterizedInstance>, ErrorCode>;

  absl::flat_hash_map<StringId, NumericVariableDefinition<BaseTypes>> numeric_definitions_;
  absl::flat_hash_map<StringId, StringVariableDefinition<BaseTypes>> string_definitions_;
  ParameterizedDefinitionsMap<NumericVariableDefinition<BaseTypes>>
      parameterized_numeric_definitions_;
  ParameterizedDefinitionsMap<StringVariableDefinition<BaseTypes>>
      parameterized_string_definitions_;
};

template <typename BaseTypes>
using VariableDefinitionsPtr = utils::NonNullSharedPtr<VariableDefinitions<BaseTypes>>;
template <typename BaseTypes>
using VariableDefinitionsConstPtr = utils::NonNullSharedPtr<const VariableDefinitions<BaseTypes>>;

}  // namespace hs::ruleset

#include <core/ruleset/variable_definition.inl>
