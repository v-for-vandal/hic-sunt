#pragma once

#include "effect_instance.hpp"

#include <stdexcept>
#include <system_error>

namespace hs::session {

template <typename BaseTypes>
EffectInstance<BaseTypes>::EffectInstance(const EffectDefinitionPtr& definition)
    : definition_(definition) {
  InitializeLuaState();
  LoadCode();
}

template <typename BaseTypes>
void EffectInstance<BaseTypes>::InitializeLuaState() {
  lua_.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table,
                      sol::lib::string);
}

template <typename BaseTypes>
void EffectInstance<BaseTypes>::LoadCode() {
  sol::load_result loaded_chunk = lua_.load(definition_->GetCode());
  if (!loaded_chunk.valid()) {
    sol::error error = loaded_chunk;
    throw std::runtime_error(error.what());
  }

  chunk_ = loaded_chunk;
}

}  // namespace hs::session
