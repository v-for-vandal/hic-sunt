#pragma once

#include <core/session/session.hpp>
#include <core/types/std_base_types.hpp>

namespace hs::session {

template <typename BaseTypes = StdBaseTypes>
class EffectExecutor {
 public:
  using ScopePtr = scope::ScopePtr<BaseTypes>;
  using EffectInstance = hs::session::EffectInstance<BaseTypes>;

  EffectExecutor() = default;

  template <typename WorldPtr, typename RuleSetPtr>
  void Execute(Session<BaseTypes, WorldPtr, RuleSetPtr>& session,
               size_t current_time);
};

}  // namespace hs::session

#include "effect_executor.inl"
