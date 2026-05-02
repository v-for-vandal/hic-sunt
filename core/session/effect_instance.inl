#pragma once

#include "effect_instance.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <string_view>

namespace hs::session {

template <typename BaseTypes>
EffectInstance<BaseTypes>::EffectInstance(const EffectDefinitionPtr& definition)
    : definition_(definition) {
  InitializeLuaState();
  auto load_result = LoadFunctions();
  if (!load_result) {
    throw std::system_error(make_error_code(load_result.error()));
  }
}

template <typename BaseTypes>
void EffectInstance<BaseTypes>::LuaHook(lua_State* lua_state,
                                        lua_Debug* /*debug_info*/) {
  lua_sethook(lua_state, nullptr, 0, 0);
  luaL_error(lua_state, "effect operation limit exceeded");
}

template <typename BaseTypes>
void EffectInstance<BaseTypes>::InitializeLuaState() {
  lua_.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table,
                      sol::lib::string);
  BindScopeChangeSet();
}

template <typename BaseTypes>
std::expected<void, ErrorCode> EffectInstance<BaseTypes>::LoadFunctions() {
  {
    const std::string source =
        "function __hic_sunt_possible(target)\n" +
        definition_->GetPossibleCode() + "\nend";
    sol::load_result loaded = lua_.load(source);
    if (!loaded.valid()) {
      return std::unexpected(ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
    }
    sol::protected_function_result result = loaded();
    if (!result.valid()) {
      return std::unexpected(ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
    }
    possible_function_ = lua_["__hic_sunt_possible"];
  }

  {
    const std::string source =
        "function __hic_sunt_effect(target)\n" +
        definition_->GetEffectCode() + "\nend";
    sol::load_result loaded = lua_.load(source);
    if (!loaded.valid()) {
      return std::unexpected(ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
    }
    sol::protected_function_result result = loaded();
    if (!result.valid()) {
      return std::unexpected(ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
    }
    effect_function_ = lua_["__hic_sunt_effect"];
  }

  return {};
}

template <typename BaseTypes>
void EffectInstance<BaseTypes>::BindScopeChangeSet() {
  lua_.new_usertype<LuaScopeChangeSet>(
      "LuaScopeChangeSet", "set_numeric_modifier",
      &LuaScopeChangeSet::SetNumericModifier, "change_numeric_modifier",
      &LuaScopeChangeSet::ChangeNumericModifier, "set_string_modifier",
      &LuaScopeChangeSet::SetStringModifier);
}

template <typename BaseTypes>
template <typename Result>
std::expected<Result, ErrorCode> EffectInstance<BaseTypes>::CallWithLimit(
    sol::protected_function& func, LuaScopeChangeSet& target,
    int max_operations) {
  if (max_operations > 0) {
    lua_sethook(lua_.lua_state(), &EffectInstance::LuaHook, LUA_MASKCOUNT,
                max_operations);
  }

  sol::protected_function_result result = func(std::ref(target));
  lua_sethook(lua_.lua_state(), nullptr, 0, 0);

  if (!result.valid()) {
    sol::error error = result;
    const std::string_view message = error.what();
    if (message.find("effect operation limit exceeded") !=
        std::string_view::npos) {
      return std::unexpected(ErrorCode::ERR_EFFECT_LUA_OPERATION_LIMIT_EXCEEDED);
    }
    return std::unexpected(ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
  }

  if constexpr (std::same_as<Result, void>) {
    return {};
  } else {
    auto opt_val = result.get<sol::optional<Result>>();
    if(!opt_val) {
      return std::unexpected(ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
    }
    return *opt_val;
  }
}

template <typename BaseTypes>
std::expected<bool, ErrorCode> EffectInstance<BaseTypes>::CheckPossible(
    ScopeChangeSet& target, int max_operations) {
  LuaScopeChangeSet wrapped_target(target, GetId());
  return CallWithLimit<bool>(possible_function_, wrapped_target, max_operations);
}

template <typename BaseTypes>
std::expected<void, ErrorCode> EffectInstance<BaseTypes>::Execute(
    ScopeChangeSet& target, int max_operations) {
  LuaScopeChangeSet wrapped_target(target, GetId());
  return CallWithLimit<void>(effect_function_, wrapped_target, max_operations);
}

}  // namespace hs::session
