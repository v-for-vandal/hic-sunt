#pragma once

#include <absl/container/flat_hash_map.h>
#include <spdlog/spdlog.h>

#include <core/types/std_base_types.hpp>
#include <core/utils/non_null_ptr.hpp>
#include <limits>
#include <utility>

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

  bool IsEmpty() const noexcept {
    return !string_definitions_.empty() || !numeric_definitions_.empty();
  }

  void Clear() {
    numeric_definitions_.clear();
    string_definitions_.clear();
  }

  void AddNumericDefinition(const StringId& id,
                            NumericVariableDefinition<BaseTypes> definition) {
    numeric_definitions_[id] = std::move(definition);
  }

  void AddStringDefinition(const StringId& id,
                           StringVariableDefinition<BaseTypes> definition) {
    string_definitions_[id] = std::move(definition);
  }

  bool IsNumericVariable(const StringId& id) const noexcept {
    return numeric_definitions_.contains(id);
  }

  bool IsStringVariable(const StringId& id) const noexcept {
    return string_definitions_.contains(id);
  }

  bool IsVariable(const StringId& id) const noexcept {
      return IsStringVariable(id) || IsNumericVariable(id);
  }

  // Finds and returns definition for variable with given id
  // Returns copy to prevent abuse.
  NumericVariableDefinition<BaseTypes> FindNumericVariable(
      const StringId& id) const {
    static NumericVariableDefinition<BaseTypes> ADHOC_VAR_DEF{};

    const auto fit = numeric_definitions_.find(id);
    if (fit == numeric_definitions_.end()) {
      spdlog::error("Variable {} is unknown or is not numeric", id);
      return ADHOC_VAR_DEF;
    }

    return fit->second;
  }

  StringVariableDefinition<BaseTypes> FindStringVariable(
      const StringId& id) const {
    static StringVariableDefinition<BaseTypes> ADHOC_VAR_DEF{};

    const auto fit = string_definitions_.find(id);
    if (fit == string_definitions_.end()) {
      spdlog::error("Variable {} is unknown or is not string", id);
      return ADHOC_VAR_DEF;
    }

    return fit->second;
  }

 private:
  absl::flat_hash_map<StringId, NumericVariableDefinition<BaseTypes>>
      numeric_definitions_;
  absl::flat_hash_map<StringId, StringVariableDefinition<BaseTypes>>
      string_definitions_;
};

template <typename BaseTypes>
using VariableDefinitionsPtr =
    utils::NonNullSharedPtr<const VariableDefinitions<BaseTypes>>;

}  // namespace hs::ruleset
