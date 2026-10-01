#pragma once

#include <absl/container/flat_hash_map.h>

#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include <expected>

#include "variable_base.hpp"

namespace hs::variables {

/** \brief Stores all modifiers that make up value of a numeric variable. */
template <typename BaseTypes = StdBaseTypes>
class NumericVariable : public VariableBase<BaseTypes> {
 public:
  using StringId = typename BaseTypes::StringId;
  using NumericValue = typename BaseTypes::NumericValue;
  using Base = VariableBase<BaseTypes>;

  std::expected<void, ErrorCode> SetModifier(const StringId& key, NumericValue add,
                                             NumericValue mult, size_t modification_time);

  std::expected<void, ErrorCode> ChangeModifier(const StringId& key, NumericValue add,
                                                NumericValue mult, size_t modification_time);

  void CalculateModifiers(NumericValue& add, NumericValue& mult) const;

  void ExplainModifiers(auto&& output_fn) const;

 private:
  struct Modifier {
    NumericValue add{0};
    NumericValue mult{0};
  };

  absl::flat_hash_map<StringId, Modifier> modifiers_;
};

}  // namespace hs::variables

#include "numeric_variable.inl"
