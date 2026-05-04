#pragma once

#include "effect_executor.hpp"

#include <system_error>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

namespace hs::session {

template <typename BaseTypes>
template <typename WorldPtr, typename RuleSetPtr>
void EffectExecutor<BaseTypes>::Execute(
    Session<BaseTypes, WorldPtr, RuleSetPtr>& session, size_t current_time) {
  struct PendingExecution {
    std::shared_ptr<EffectInstance> effect;
    ScopePtr scope;
  };

  std::vector<PendingExecution> pending_executions;

  for (const auto& effect : session.effects_) {
    const auto scope_type = effect->GetDefinition()->GetScopeType();
    const auto scopes_it = session.scopes_by_type_.find(scope_type);
    if (scopes_it == session.scopes_by_type_.end()) {
      continue;
    }

    const auto& dependencies = effect->GetDefinition()->GetDependencies();
    for (const auto& scope : scopes_it->second) {
      bool has_recent_dependency = false;
      for (const auto& dependency : dependencies) {
        auto modification_time = scope->GetModificationTime(dependency);
        if (!modification_time) {
          continue;
        }

        if (*modification_time >= current_time) {
          has_recent_dependency = true;
          break;
        }
      }

      if (!has_recent_dependency) {
        continue;
      }

      auto possible_result = effect->CheckPossible(scope);
      if (!possible_result) {
        spdlog::warn(
            "Failed to check possible for effect {} on scope {}: {}",
            effect->GetId(),
            scope->GetId(),
            make_error_code(possible_result.error()).message());
        continue;
      }

      if (!*possible_result) {
        continue;
      }

      pending_executions.push_back(PendingExecution{
          .effect = effect,
          .scope = scope,
      });
    }
  }

  for (const auto& pending_execution : pending_executions) {
    auto execute_result = pending_execution.effect->Execute(
        pending_execution.scope);
    if (!execute_result) {
      throw std::system_error(make_error_code(execute_result.error()));
    }

    for (const auto& changes : *execute_result) {
      auto apply_result = changes.Apply(current_time);
      if (!apply_result) {
        throw std::system_error(make_error_code(apply_result.error()));
      }
    }
  }
}

}  // namespace hs::session
