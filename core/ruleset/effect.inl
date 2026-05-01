#pragma once

#include "effect.hpp"

namespace hs::ruleset {

template <typename BaseTypes>
auto EffectDefinition<BaseTypes>::GetId() const noexcept -> const StringId& {
  return id_;
}

}  // namespace hs::ruleset
