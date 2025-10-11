#pragma once

#include <absl/container/flat_hash_map.h>
#include <scope/scope.pb.h>

#include <core/types/std_base_types.hpp>

namespace hs::scope {

/** \brief This class stores all modifires that make up value of this variable
 *
 * However, to get proper value, one need to take into account modifiers from
 * parent scope and so on.
 */
template <typename BaseTypes = StdBaseTypes>
class NumericVariable {
 public:
  using StringId = typename BaseTypes::StringId;
  using NumericValue = typename BaseTypes::NumericValue;

  bool AddModifiers(const StringId& key, NumericValue add, NumericValue mult);

  /** \brief Get total addictive and multiplicative modifiers for this
   *  variable. It only works within this scope.
   */
  void CalculateModifiers(NumericValue& add, NumericValue& mult) const;

  /** \brief get all modifiers in an explainable way
   * Should be used for UI purposes
   */
  void ExplainModifiers(auto&& output_fn) const;

 private:
  struct Modifier {
    NumericValue add;
    NumericValue mult;
  };

  absl::flat_hash_map<StringId, Modifier> modifiers_;
};

}  // namespace hs::scope

#include "numeric_variable.inl"
