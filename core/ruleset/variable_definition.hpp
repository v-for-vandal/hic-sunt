#pragma once

#include <absl/container/flat_hash_map.h>
#include <spdlog/spdlog.h>

#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/non_null_ptr.hpp>
#include <expected>
#include <limits>
#include <utility>

#include "core/types/variable_type.hpp"

namespace hs::ruleset {

template <typename BaseTypes = StdBaseTypes>
class NumericVariableDefinition {
 public:
  using NumericValue = typename BaseTypes::NumericValue;

  NumericValue maximum = std::numeric_limits<NumericValue>::max();
  NumericValue minimum = std::numeric_limits<NumericValue>::lowest();
};

template <typename BaseTypes = StdBaseTypes>
class StringVariableDefinition {
 public:
  using StringId = typename BaseTypes::StringId;

  StringId default_value{};
};

template <typename BaseTypes = StdBaseTypes>
class VariableDefinitions {
 public:
  using StringId = typename BaseTypes::StringId;
  using VariableType = types::VariableType;

  bool IsEmpty() const noexcept {
    return string_definitions_.empty() && numeric_definitions_.empty();
  }

  void Clear() {
    numeric_definitions_.clear();
    string_definitions_.clear();
  }

  std::expected<void, ErrorCode> AddNumericDefinition(
      const StringId& id, NumericVariableDefinition<BaseTypes> definition) {
    if (const auto var_type = GetVariableType(id);
        var_type != VariableType::kNumeric && var_type != VariableType::kMissing) {
      return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
    }

    numeric_definitions_[id] = std::move(definition);
    return {};
  }

  std::expected<void, ErrorCode> AddStringDefinition(
      const StringId& id, StringVariableDefinition<BaseTypes> definition) {
    if (const auto var_type = GetVariableType(id);
        var_type != VariableType::kString && var_type != VariableType::kMissing) {
      return std::unexpected(ErrorCode::ERR_INCORRECT_VARIABLE_TYPE);
    }

    string_definitions_[id] = std::move(definition);
    return {};
  }

  bool IsNumericVariable(const StringId& id) const noexcept {
    return numeric_definitions_.contains(id);
  }

  bool IsStringVariable(const StringId& id) const noexcept {
    return string_definitions_.contains(id);
  }

  bool IsVariable(const StringId& id) const noexcept {
    return GetVariableType(id) != VariableType::kMissing;
  }

  VariableType GetVariableType(const StringId& id) const noexcept {
    if (numeric_definitions_.contains(id)) {
      return VariableType::kNumeric;
    }
    if (string_definitions_.contains(id)) {
      return VariableType::kString;
    }
    return VariableType::kMissing;
  }

  // Finds and returns definition for variable with given id
  // Returns copy to prevent abuse.
  std::expected<NumericVariableDefinition<BaseTypes>, ErrorCode> FindNumericVariable(
      const StringId& id) const {
    const auto fit = numeric_definitions_.find(id);
    if (fit == numeric_definitions_.end()) {
      spdlog::error("Variable {} is unknown or is not numeric", id);
      return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
    }

    return fit->second;
  }

  const auto& GetNumericDefinitions() const noexcept { return numeric_definitions_; }

  const auto& GetStringDefinitions() const noexcept { return string_definitions_; }

  std::expected<StringVariableDefinition<BaseTypes>, ErrorCode> FindStringVariable(
      const StringId& id) const {
    const auto fit = string_definitions_.find(id);
    if (fit == string_definitions_.end()) {
      spdlog::error("Variable {} is unknown or is not string", id);
      return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
    }

    return fit->second;
  }

 private:
  absl::flat_hash_map<StringId, NumericVariableDefinition<BaseTypes>> numeric_definitions_;
  absl::flat_hash_map<StringId, StringVariableDefinition<BaseTypes>> string_definitions_;
};

template <typename BaseTypes>
using VariableDefinitionsPtr = utils::NonNullSharedPtr<VariableDefinitions<BaseTypes>>;
template <typename BaseTypes>
using VariableDefinitionsConstPtr = utils::NonNullSharedPtr<const VariableDefinitions<BaseTypes>>;

}  // namespace hs::ruleset
