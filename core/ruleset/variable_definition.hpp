#pragma once

#include <core/types/std_base_types.hpp>
#include <core/utils/non_null_ptr.hpp>

#include <spdlog/spdlog.h>
#include <absl/container/flat_hash_map.h>

namespace hs::ruleset {

template <typename BaseTypes = StdBaseTypes>
class NumericVariableDefinition {
  using NumericValue = typename BaseTypes::NumericValue;

  NumericValue maximum = std::numeric_limits<NumericValue>::maximum();
  NumericValue minimum = std::numeric_limits<NumericValue>::lowest();
};



template <typename BaseTypes = StdBaseTypes>
class VariableDefinitions {
  public:
    using StringId = typename BaseTypes::StringId;

    // Finds and returns definition for variable with given id
    // Returns copy to prevent abuse.
    NumericVariableDefinition<BaseTypes> FindNumericVariable(
      const StringId& id) const {

      static NumericVariableDefinition<BaseTypes> ADHOC_VAR_DEF{};

      const auto fit = numeric_definitions_->find(id);
      if (fit == numeric_definitions_.end()) {
        spdlog::error("Variable {} is unknown or is not numeric", id);
        return ADHOC_VAR_DEF;
      }

      return fit->second;
    }

  private:
    absl::flat_hash_map<StringId, NumericVariableDefinition<BaseTypes>> numeric_definitions_;
};

template <typename BaseTypes>
using VariableDefinitionsPtr = utils::NonNullSharedPtr<const VariableDefinitions<BaseTypes>>;

}
