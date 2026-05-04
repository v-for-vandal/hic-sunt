#pragma once

#include <core/session/session.hpp>
#include <core/types/std_base_types.hpp>

namespace hs::session {

template <typename BaseTypes = StdBaseTypes>
class EffectExecutor {
 public:
  using Session = hs::session::Session<BaseTypes>;
  using ScopePtr = scope::ScopePtr<BaseTypes>;
  using EffectInstance = hs::session::EffectInstance<BaseTypes>;

  EffectExecutor() = default;

  void Execute(Session& session, size_t current_time);
};

}  // namespace hs::session

#include "effect_executor.inl"
