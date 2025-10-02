#pragma once

#include <spdlog/spdlog.h>

namespace hs::scope {

template <typename BaseTypes>
void NumericVariable<BaseTypes>::FillModifiers(NumericValue& add, NumericValue& mult) const {
    for(auto& [_,v] : add_modifiers_) {
      add += v;
    }

    for(auto& [_,v] : mult_modifiers_) {
      mult+= v;
    }
}

template <typename BaseTypes>
void NumericVariable<BaseTypes>::AddModifiers(const StringId &key,
                       NumericValue add, NumericValue mult)
{
    if (add != 0) {
        add_modifiers_[key] = add;
    } else {
        // remove it
        add_modifiers_.erase(key);
    }

    if (mult != 0) {
        mult_modifiers_[key] = mult;
    } else {
        // remove it
        mult_modifiers_.erase(key);
    }
}

}
