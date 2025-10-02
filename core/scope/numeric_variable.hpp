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

  void AddModifiers(const StringId &key,
                       NumericValue add, NumericValue mult);

  /** \brief Get total addictive and multiplicative modifiers for this
   *  variable. It only works within this scope.
   */
  void FillModifiers(NumericValue& add, NumericValue& mult) const;


private:
  absl::flat_hash_map<StringId, NumericValue> add_modifiers_;
  absl::flat_hash_map<StringId, NumericValue> mult_modifiers_;
};

}

#include "numeric_variable.inl"
