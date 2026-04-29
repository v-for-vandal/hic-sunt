#pragma once

#include <absl/container/flat_hash_map.h>
#include <scope/scope.pb.h>

#include <core/types/std_base_types.hpp>
#include <core/types/error_code.hpp>
#include "variable_base.hpp"

#include <expected>

namespace hs::scope {

/*! \brief String variable holds string value. Modifiers determine what value
 * will be in the end. Modifier with higher level wins. For equal level, order is
 * based on key comparison.
 */
template <typename BaseTypes = StdBaseTypes>
class StringVariable : public VariableBase<BaseTypes> {
 public:
  using String = typename BaseTypes::String;
  using StringId = typename BaseTypes::StringId;
  using NumericValue = typename BaseTypes::NumericValue;

  /*! \brief Sets modifier with given key to given value.
   *
   * Parameter \p level will determine what is the final result of the variable
   */
  std::expected<void, ErrorCode> SetModifier(const StringId& key, const StringId& val,
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

}  // namespace hs::scope

#include "string_variable.inl"
