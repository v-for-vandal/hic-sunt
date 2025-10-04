#pragma once

#include <spdlog/spdlog.h>

namespace hs::scope {

template <typename BaseTypes>
bool StringVariable<BaseTypes>::AddModifiers(const StringId& key, const StringId& val,
    NumericValue level)
{
    if(BaseTypes::IsNullToken(key)) {
      spdlog::error("Empty modifier key is not allowed");
      return false;
    }
    if (level != 0) {
        modifiers_[key] = Modifier{.value=val, .level=level};
    } else {
        if (!BaseTypes::IsNullToken(key)) {
            spdlog::debug(
                "Passing non-empty value {} with level == 0 is useless as this value will never be used - zero means remove-this-modifier");
        }
        modifiers_.erase(key);
    }
    return true;
}

template <typename BaseTypes>
void StringVariable<BaseTypes>::CalculateModifiers(StringId& value, NumericValue& level) {
    for (auto& [k, v] : modifiers_) {
        if (v.level > level) {
            level = v.level;
            value = v.value;
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
