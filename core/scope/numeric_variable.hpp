#pragma once

#include <absl/container/flat_hash_map.h>
#include <scope/scope.pb.h>

#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include <expected>

#include "variable_base.hpp"

namespace hs::scope {

/** \brief This class stores all modifires that make up value of this variable
 *
 * However, to get proper value, one need to take into account modifiers from
 * parent scope and so on.
 */
template <typename BaseTypes = StdBaseTypes>
class NumericVariable : public VariableBase<BaseTypes>{
 public:
  using StringId = typename BaseTypes::StringId;
  using NumericValue = typename BaseTypes::NumericValue;
  using Base = VariableBase<BaseTypes>;

  /** \brief Creates or replaces modifier with given key with new values.
   *
   * If no such modifier exists, new one will be created
   * if \p modification_time is not 0, then variable modification time will be updated. This
   * will be later used for cache invalidation and to recalculate effects and modifiers.
   * If \p modification_time is less than current modification time, it will be ignored and log
   * message will be written.
   */
  std::expected<void, ErrorCode> SetModifier(const StringId& key, NumericValue add, NumericValue mult,
      size_t modification_time);

  /** \brief Changes value of given modifier.
   *
   * Provided values \p add and \p mult will be **added** to currently present value.
   * If no such modifier exists, new one will be created, initialized with zeroes
   * and change will be applied to it.
   * if \p modification_time is not 0, then variable modification time will be updated. This
   * will be later used for cache invalidation and to recalculate effects and modifiers
   * If \p modification_time is less than current modification time, it will be ignored and log
   * message will be written.
   */
  std::expected<void, ErrorCode> ChangeModifier(const StringId& key, NumericValue add, NumericValue mult,
      size_t modification_time);

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
    NumericValue add{0};
    NumericValue mult{0};
  };

  absl::flat_hash_map<StringId, Modifier> modifiers_;
};

}  // namespace hs::scope

#include "numeric_variable.inl"
