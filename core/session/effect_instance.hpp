#pragma once

#include <sol/sol.hpp>

#include <core/ruleset/effect.hpp>
#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/non_null_ptr.hpp>
#include <expected>
#include <string_view>

namespace hs::session {

template <typename BaseTypes = StdBaseTypes>
class EffectInstance {
 public:
  using EffectDefinition = ruleset::EffectDefinition<BaseTypes>;
  using EffectDefinitionPtr = ruleset::ConstEffectDefinitionPtr<BaseTypes>;
  using StringId = typename BaseTypes::StringId;

  explicit EffectInstance(const EffectDefinitionPtr& definition);

  const EffectDefinitionPtr& GetDefinition() const noexcept {
    return definition_;
  }
  const sol::state& GetLuaState() const noexcept { return lua_; }
  sol::state& GetLuaState() noexcept { return lua_; }
  const sol::protected_function& GetChunk() const noexcept { return chunk_; }
  const StringId& GetId() const { return definition_->GetId(); }
  std::string_view GetCode() const noexcept { return definition_->GetCode(); }

 private:
  void InitializeLuaState();
  void LoadCode();

  EffectDefinitionPtr definition_;
  sol::state lua_;
  sol::protected_function chunk_;
};

}  // namespace hs::session

#include "effect_instance.inl"
