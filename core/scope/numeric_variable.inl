#pragma once

#include <spdlog/spdlog.h>

namespace hs::scope {

template <typename BaseTypes>
void NumericVariable<BaseTypes>::CalculateModifiers(NumericValue& add, NumericValue& mult) const {
    for(auto& [_,v] : modifiers_) {
      add += v.add;
      mult += v.mult;
    }
}

template <typename BaseTypes>
void NumericVariable<BaseTypes>::ExplainModifiers(auto&& output_fn) const {
    for(auto& [k,v] : modifiers_) {
        output_fn(k, v.add, v.mult);
    }

}

template <typename BaseTypes>
bool NumericVariable<BaseTypes>::AddModifiers(const StringId &key,
                       NumericValue add, NumericValue mult)
{
    if(BaseTypes::IsNullToken(key)) {
      spdlog::error("Empty modifier key is not allowed");
      return false;
    }
    modifiers_[key] = Modifier{.add=add, .mult=mult};

    return true;
}

}
