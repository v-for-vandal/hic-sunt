#pragma once

#include <sol/sol.hpp>

#include <core/ruleset/effect.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/non_null_ptr.hpp>

namespace hs::session {

template <typename BaseTypes = StdBaseTypes>
class EffectInstance {
 public:
  using EffectDefinition = ruleset::EffectDefinition<BaseTypes>;
  using EffectDefinitionPtr = ruleset::ConstEffectDefinitionPtr<BaseTypes>;
  using StringId = typename BaseTypes::StringId;

  explicit EffectInstance(const EffectDefinitionPtr& definition);

  const EffectDefinitionPtr& GetDefinition() const noexcept { return definition_; }
  const auto& GetLuaState() const noexcept { return lua_; }
  auto& GetLuaState() noexcept { return lua_; }
  StringId GetId() const { return definition_->GetId(); }

 private:
  EffectDefinitionPtr definition_;
  sol::state lua_;
};

}  // namespace hs::session

#include "effect_instance.inl"
