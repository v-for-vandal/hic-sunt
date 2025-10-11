#pragma once

#include <absl/container/flat_hash_map.h>
#include <scope/scope.pb.h>
#include <core/types/std_base_types.hpp>

namespace hs::scope {

template <typename BaseTypes = StdBaseTypes>
class StringVariable {
public:
  using String = typename BaseTypes::String;
  using StringId = typename BaseTypes::StringId;
  using NumericValue = typename BaseTypes::NumericValue;

  bool AddModifiers(const StringId& key, const StringId& val,
    NumericValue level);

  // Method takes current level as input, and if this variable has
  // value with higher level, it will overwrite both value and level
  void CalculateModifiers(StringId& value, NumericValue& level);

  /** \brief get all modifiers in an explainable way
   * Should be used for UI purposes
   */
  void ExplainModifiers(auto&& output_fn) const;

private:
  struct Modifier {
      StringId value;
      NumericValue level;
  };
  absl::flat_hash_map<StringId, Modifier> modifiers_;

};

}

#include "string_variable.inl"
