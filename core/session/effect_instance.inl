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
      // TODO: pass __var_X as function arguments and not as global bound variables
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
std::expected<void, ErrorCode> EffectInstance<BaseTypes>::BindVariables(
    const ScopePtr& scope) {
  for (const auto& lua_var : definition_->GetLuaVariables()) {
    if (scope->IsNumericVariable(lua_var.variable_id)) {
      auto value = scope->GetNumericValue(lua_var.variable_id);
      if (!value) {
        return std::unexpected(value.error());
      }
      lua_[lua_var.lua_name] = *value;
    } else if (scope->IsStringVariable(lua_var.variable_id)) {
      auto value = scope->GetStringValue(lua_var.variable_id);
      if (!value) {
        return std::unexpected(value.error());
      }
      lua_[lua_var.lua_name] = *value;
    } else {
      return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
    }
  }

  return {};
}

template <typename BaseTypes>
template <typename Result, typename... Args>
std::expected<Result, ErrorCode> EffectInstance<BaseTypes>::CallWithLimit(
    sol::protected_function& func, int max_operations, Args&&... args) {
  if (max_operations > 0) {
    lua_sethook(lua_.lua_state(), &EffectInstance::LuaHook, LUA_MASKCOUNT,
                max_operations);
  }

  sol::protected_function_result result = func(std::forward<Args>(args)...);
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
    if (!opt_val) {
      return std::unexpected(ErrorCode::ERR_EFFECT_LUA_RUNTIME_ERROR);
    }
    return *opt_val;
  }
}

template <typename BaseTypes>
std::expected<bool, ErrorCode> EffectInstance<BaseTypes>::CheckPossible(
    const ScopePtr& scope, int max_operations) {
  auto bind_result = BindVariables(scope);
  if (!bind_result) {
    return std::unexpected(bind_result.error());
  }

  ScopeChangeSet changes(scope);
  LuaScopeChangeSet target(changes, GetId());
  return CallWithLimit<bool>(possible_function_, max_operations,
                             std::ref(target));
}

template <typename BaseTypes>
std::expected<std::vector<typename EffectInstance<BaseTypes>::ScopeChangeSet>, ErrorCode>
EffectInstance<BaseTypes>::Execute(const ScopePtr& scope, int max_operations) {
  auto bind_result = BindVariables(scope);
  if (!bind_result) {
    return std::unexpected(bind_result.error());
  }

  std::vector<ScopeChangeSet> result_changes;
  result_changes.emplace_back(scope);
  LuaScopeChangeSet target(result_changes.back(), GetId());

  auto exec_result =
      CallWithLimit<void>(effect_function_, max_operations, std::ref(target));
  if (!exec_result) {
    return std::unexpected(exec_result.error());
  }

  return result_changes;
}

}  // namespace hs::session
