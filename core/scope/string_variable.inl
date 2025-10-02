#pragma once

#include <spdlog/spdlog.h>

namespace hs::scope {

template <typename BaseTypes>
void StringVariable<BaseTypes>::AddModifiers(const StringId& key, const StringId& val,
    NumericValue level)
{
    if (level != 0) {
        modifiers_[key] = std::make_pair<NumericValue, StringId>(level, val);
    } else {
        if (!BaseTypes::IsNullToken(key)) {
            spdlog::debug(
                "Passing non-empty value {} with level == 0 is useless as this value will never be used - zero means remove-this-modifier");
        }
        modifiers_.erase(key);
    }
}

template <typename BaseTypes>
void StringVariable<BaseTypes>::FillModifiers(StringId& value, NumericValue& level) {
    for (auto& [k, v] : modifiers_) {
        if (v.first > level) {
            level = v.first;
            value = v.second;
        }
    }
}

}
