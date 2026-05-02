#pragma once

#include "lua_scope_change_set.hpp"

#include <fmt/format.h>

namespace hs::session {

template <typename BaseTypes>
sol::error LuaScopeChangeSet<BaseTypes>::MakeLuaError(ErrorCode error) {
  return sol::error(fmt::format("{}", make_error_code(error).message()));
}

template <typename BaseTypes>
void LuaScopeChangeSet<BaseTypes>::SetNumericModifier(const StringId& variable,
                                                      NumericValue add,
                                                      NumericValue mult) {
  auto result = target_.SetNumericModifier(variable, key_, add, mult);
  if (!result) {
    throw MakeLuaError(result.error());
  }
}

template <typename BaseTypes>
void LuaScopeChangeSet<BaseTypes>::ChangeNumericModifier(
    const StringId& variable, NumericValue add, NumericValue mult) {
  auto result = target_.ChangeNumericModifier(variable, key_, add, mult);
  if (!result) {
    throw MakeLuaError(result.error());
  }
}

template <typename BaseTypes>
void LuaScopeChangeSet<BaseTypes>::SetStringModifier(const StringId& variable,
                                                     const StringId& value,
                                                     NumericValue level) {
  auto result = target_.SetStringModifier(variable, key_, value, level);
  if (!result) {
    throw MakeLuaError(result.error());
  }
}

}  // namespace hs::session
