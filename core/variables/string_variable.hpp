#pragma once

#include <absl/container/flat_hash_map.h>

#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include <expected>

#include "variable_base.hpp"

namespace hs::variables {

/** \brief String variable. Modifiers determine final value; higher level wins. */
template <typename BaseTypes = StdBaseTypes>
class StringVariable : public VariableBase<BaseTypes> {
 public:
  using String = typename BaseTypes::String;
  using StringId = typename BaseTypes::StringId;
  using NumericValue = typename BaseTypes::NumericValue;
  using Base = VariableBase<BaseTypes>;

  std::expected<void, ErrorCode> SetModifier(const StringId& key, const StringId& value,
                                             NumericValue level, size_t modification_time);

  void CalculateModifiers(StringId& value, NumericValue& level) const;

  void MergeModifiers(StringId& value, NumericValue& level, StringId& key) const;

  void ExplainModifiers(auto&& output_fn) const;

 private:
  struct Modifier {
    StringId value;
    NumericValue level;
  };

  absl::flat_hash_map<StringId, Modifier> modifiers_;
};

}  // namespace hs::variables

#include "string_variable.inl"
