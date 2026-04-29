#pragma once

#include "string_variable.hpp"

#include <spdlog/spdlog.h>
#include <expected>

namespace hs::scope {

template <typename BaseTypes>
std::expected<void, ErrorCode> StringVariable<BaseTypes>::SetModifier(const StringId& key, const StringId& val,
    NumericValue level)
{
    if(BaseTypes::IsNullToken(key)) {
      spdlog::error("Empty modifier key is not allowed");
      return std::unexpected(ErrorCode::ERR_EMPTY_MODIFIER_KEY);
    }

    modifiers_[key] = Modifier{.value=val, .level=level};
    return {};
}

template <typename BaseTypes>
void StringVariable<BaseTypes>::CalculateModifiers(StringId& value, NumericValue& level) {
    if (modifiers_.empty()) {
        value = StringId{};
        level = 0;
        return;
    }
    value = modifiers_.begin()->second.value;
    level = modifiers_.begin()->second.level;
    StringId current_key = modifiers_.begin()->first;
    for (auto& [k, v] : modifiers_) {
        // high level wins. In case of impasse, compare keys
        if ((v.level > level) || (v.level == level && k > current_key)) {
            level = v.level;
            value = v.value;
            current_key = k;
        }
    }
}

template <typename BaseTypes>
void StringVariable<BaseTypes>::ExplainModifiers(auto&& output_fn) const {
    for(auto& [k,v] : modifiers_) {
        output_fn(k, v.value, v.level);
    }
}

}
