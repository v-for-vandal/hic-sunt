#pragma once

#include <sol/sol.hpp>

#include <core/ruleset/effect.hpp>
#include <core/scope/scope_change_set.hpp>
#include <core/session/lua_scope_change_set.hpp>
#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include <expected>

namespace hs::session {

template <typename BaseTypes = StdBaseTypes>
class EffectInstance {
 public:
  using EffectDefinition = ruleset::EffectDefinition<BaseTypes>;
  using EffectDefinitionPtr = ruleset::ConstEffectDefinitionPtr<BaseTypes>;
  using Scope = scope::Scope<BaseTypes>;
  using ScopePtr = scope::ScopePtr<BaseTypes>;
  using ScopeChangeSet = scope::ScopeChangeSet<BaseTypes>;
  using LuaScopeChangeSet = session::LuaScopeChangeSet<BaseTypes>;
  using StringId = typename BaseTypes::StringId;

  explicit EffectInstance(const EffectDefinitionPtr& definition);

  const EffectDefinitionPtr& GetDefinition() const noexcept {
    return definition_;
  }
  const StringId& GetId() const { return definition_->GetId(); }

  std::expected<bool, ErrorCode> CheckPossible(const ScopePtr& scope,
                                               int max_operations);
  std::expected<std::vector<ScopeChangeSet>, ErrorCode> Execute(
      const ScopePtr& scope, int max_operations);

 private:
  static void LuaHook(lua_State* lua_state, lua_Debug* debug_info);

  void InitializeLuaState();
  std::expected<void, ErrorCode> LoadFunctions();
  void BindScopeChangeSet();
  std::expected<void, ErrorCode> BindVariables(const ScopePtr& scope);

  template <typename Result, typename... Args>
  std::expected<Result, ErrorCode> CallWithLimit(sol::protected_function& func,
                                                 int max_operations,
                                                 Args&&... args);

  EffectDefinitionPtr definition_;
  sol::state lua_;
  sol::protected_function possible_function_;
  sol::protected_function effect_function_;
};

}  // namespace hs::session

#include "effect_instance.inl"
