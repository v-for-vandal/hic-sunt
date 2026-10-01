#pragma once

#include <spdlog/spdlog.h>

#include <expected>

#include "string_variable.hpp"

namespace hs::variables {

template <typename BaseTypes>
std::expected<void, ErrorCode> StringVariable<BaseTypes>::SetModifier(const StringId& key,
                                                                      const StringId& value,
                                                                      NumericValue level,
                                                                      size_t modification_time) {
  if (BaseTypes::IsNullToken(key)) {
    spdlog::error("Empty modifier key is not allowed");
    return std::unexpected(ErrorCode::ERR_EMPTY_MODIFIER_KEY);
  }

  modifiers_[key] = Modifier{.value = value, .level = level};
  Base::UpdateModificationTime(modification_time);
  return {};
}

template <typename BaseTypes>
void StringVariable<BaseTypes>::CalculateModifiers(StringId& value, NumericValue& level) const {
  if (modifiers_.empty()) {
    value = StringId{};
    level = 0;
    return;
  }

  value = modifiers_.begin()->second.value;
  level = modifiers_.begin()->second.level;
  StringId key = modifiers_.begin()->first;
  MergeModifiers(value, level, key);
}

template <typename BaseTypes>
void StringVariable<BaseTypes>::MergeModifiers(StringId& value, NumericValue& level,
                                               StringId& current_key) const {
  for (const auto& [key, modifier] : modifiers_) {
    // High level wins. If levels are equal, compare keys. Real modifier keys
    // cannot be null, so any level-0 modifier wins over a default value that
    // uses a null key as sentinel.
    if ((modifier.level > level) || (modifier.level == level && key > current_key)) {
      level = modifier.level;
      value = modifier.value;
      current_key = key;
    }
  }
}

template <typename BaseTypes>
void StringVariable<BaseTypes>::ExplainModifiers(auto&& output_fn) const {
  for (const auto& [key, value] : modifiers_) {
    output_fn(key, value.value, value.level);
  }
}

}  // namespace hs::variables
