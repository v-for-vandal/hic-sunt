#pragma once

#include <spdlog/spdlog.h>

#include <expected>

#include "numeric_variable.hpp"

namespace hs::variables {

template <typename BaseTypes>
void NumericVariable<BaseTypes>::CalculateModifiers(NumericValue& add, NumericValue& mult) const {
  for (const auto& [_, value] : modifiers_) {
    add += value.add;
    mult += value.mult;
  }
}

template <typename BaseTypes>
void NumericVariable<BaseTypes>::ExplainModifiers(auto&& output_fn) const {
  for (const auto& [key, value] : modifiers_) {
    output_fn(key, value.add, value.mult);
  }
}

template <typename BaseTypes>
std::expected<void, ErrorCode> NumericVariable<BaseTypes>::SetModifier(const StringId& key,
                                                                       NumericValue add,
                                                                       NumericValue mult,
                                                                       size_t modification_time) {
  if (BaseTypes::IsNullToken(key)) {
    spdlog::error("Empty modifier key is not allowed");
    return std::unexpected(ErrorCode::ERR_EMPTY_MODIFIER_KEY);
  }
  modifiers_[key] = Modifier{.add = add, .mult = mult};

  Base::UpdateModificationTime(modification_time);

  return {};
}

template <typename BaseTypes>
std::expected<void, ErrorCode> NumericVariable<BaseTypes>::ChangeModifier(
    const StringId& key, NumericValue add, NumericValue mult, size_t modification_time) {
  if (BaseTypes::IsNullToken(key)) {
    spdlog::error("Empty modifier key is not allowed");
    return std::unexpected(ErrorCode::ERR_EMPTY_MODIFIER_KEY);
  }
  modifiers_[key].add += add;
  modifiers_[key].mult += mult;

  Base::UpdateModificationTime(modification_time);

  return {};
}

}  // namespace hs::variables
