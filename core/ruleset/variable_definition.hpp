#pragma once

#include <absl/container/flat_hash_map.h>
#include <spdlog/spdlog.h>

#include <core/types/error_code.hpp>
#include <core/types/scope_type.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/non_null_ptr.hpp>
#include <expected>
#include <limits>
#include <ruleset/variables.pb.h>
#include <variant>

#include "core/types/variable_type.hpp"

namespace hs::ruleset {

template <typename BaseTypes = StdBaseTypes>
class VariableDefinitionBase {
 public:
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
class VariableDefinitions {
 public:
  using StringId = typename BaseTypes::StringId;
  using VariableType = types::VariableType;
  using ParsedVariableDefinition =
      std::variant<NumericVariableDefinition<BaseTypes>, StringVariableDefinition<BaseTypes>,
                   ErrorCode>;

  static ParsedVariableDefinition ParseFromProto(const proto::ruleset::Variable &definition);

  bool IsEmpty() const noexcept {
    return string_definitions_.empty() && numeric_definitions_.empty();
  }

  void Clear() {
    numeric_definitions_.clear();
    string_definitions_.clear();
  }

  std::expected<void, ErrorCode> AddNumericDefinition(
      const StringId& id, NumericVariableDefinition<BaseTypes> definition);

  std::expected<void, ErrorCode> AddStringDefinition(
      const StringId& id, StringVariableDefinition<BaseTypes> definition);

  bool IsNumericVariable(const StringId& id) const noexcept {
    return numeric_definitions_.contains(id);
  }

  bool IsStringVariable(const StringId& id) const noexcept {
    return string_definitions_.contains(id);
  }

  bool IsVariable(const StringId& id) const noexcept {
    return GetVariableType(id) != VariableType::kMissing;
  }

  VariableType GetVariableType(const StringId& id) const noexcept;

  std::expected<NumericVariableDefinition<BaseTypes>, ErrorCode> FindNumericVariable(
      const StringId& id) const;

  const auto& GetNumericDefinitions() const noexcept { return numeric_definitions_; }

  const auto& GetStringDefinitions() const noexcept { return string_definitions_; }

  std::expected<StringVariableDefinition<BaseTypes>, ErrorCode> FindStringVariable(
      const StringId& id) const;

 private:
  absl::flat_hash_map<StringId, NumericVariableDefinition<BaseTypes>> numeric_definitions_;
  absl::flat_hash_map<StringId, StringVariableDefinition<BaseTypes>> string_definitions_;
};

template <typename BaseTypes>
using VariableDefinitionsPtr = utils::NonNullSharedPtr<VariableDefinitions<BaseTypes>>;
template <typename BaseTypes>
using VariableDefinitionsConstPtr = utils::NonNullSharedPtr<const VariableDefinitions<BaseTypes>>;

}  // namespace hs::ruleset

#include <core/ruleset/variable_definition.inl>
