#pragma once

#include "numeric_variable.hpp"

#include <spdlog/spdlog.h>
#include <expected>

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
std::expected<void, ErrorCode> NumericVariable<BaseTypes>::SetModifier(const StringId &key,
                       NumericValue add, NumericValue mult, size_t modification_time)
{
    if(BaseTypes::IsNullToken(key)) {
      spdlog::error("Empty modifier key is not allowed");
      return std::unexpected(ErrorCode::ERR_EMPTY_MODIFIER_KEY);
    }
    modifiers_[key] = Modifier{.add=add, .mult=mult};

    Base::UpdateModificationTime(modification_time);

    return {};
}

template <typename BaseTypes>
std::expected<void, ErrorCode> NumericVariable<BaseTypes>::ChangeModifier(const StringId &key,
                       NumericValue add, NumericValue mult, size_t modification_time)
{
    if(BaseTypes::IsNullToken(key)) {
      spdlog::error("Empty modifier key is not allowed");
      return std::unexpected(ErrorCode::ERR_EMPTY_MODIFIER_KEY);
    }
    modifiers_[key].add += add;
    modifiers_[key].mult += mult;

    Base::UpdateModificationTime(modification_time);

    return {};
}

}
