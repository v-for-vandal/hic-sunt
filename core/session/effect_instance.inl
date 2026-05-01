#pragma once

#include "effect_instance.hpp"

namespace hs::session {

template <typename BaseTypes>
EffectInstance<BaseTypes>::EffectInstance(const EffectDefinitionPtr& definition)
    : definition_(definition) {}

}  // namespace hs::session
