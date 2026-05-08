#pragma once

#include "effect_executor.hpp"

#include <chrono>
#include <system_error>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

namespace hs::session {

template <typename BaseTypes>
void EffectExecutionStatistics<BaseTypes>::RecordExecution(
    const StringId& effect_id, size_t duration_ns, size_t change_count) {
  auto& entry = entries_[effect_id];
  ++entry.execution_count;
  entry.total_duration_ns += duration_ns;
  entry.total_changes += change_count;
}

template <typename BaseTypes>
void EffectExecutionStatistics<BaseTypes>::RecordFailure(
    const StringId& effect_id) {
  ++entries_[effect_id].failure_count;
}

template <typename BaseTypes>
void EffectExecutionStatistics<BaseTypes>::MergeFrom(
    const EffectExecutionStatistics& other) {
  for (const auto& [effect_id, entry] : other.entries_) {
    entries_[effect_id] += entry;
  }
}

template <typename BaseTypes>
template <typename WorldPtr, typename RuleSetPtr>
auto EffectExecutor<BaseTypes>::Execute(
    Session<BaseTypes, WorldPtr, RuleSetPtr>& session, size_t current_time)
    -> Statistics {
  struct PendingExecution {
    std::shared_ptr<EffectInstance> effect;
    ScopePtr scope;
  };

  std::vector<PendingExecution> pending_executions;
  Statistics statistics;

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
        statistics.RecordFailure(effect->GetId());
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
    const auto started_at = std::chrono::steady_clock::now();
    auto execute_result = pending_execution.effect->Execute(
        pending_execution.scope);
    const auto finished_at = std::chrono::steady_clock::now();

    if (!execute_result) {
      statistics.RecordFailure(pending_execution.effect->GetId());
      throw std::system_error(make_error_code(execute_result.error()));
    }

    size_t total_changes = 0;
    for (const auto& changes : *execute_result) {
      total_changes += changes.GetOperationCount();
      auto apply_result = changes.Apply(current_time);
      if (!apply_result) {
        statistics.RecordFailure(pending_execution.effect->GetId());
        throw std::system_error(make_error_code(apply_result.error()));
      }
    }

    const auto duration_ns = static_cast<size_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            finished_at - started_at)
            .count());
    statistics.RecordExecution(pending_execution.effect->GetId(), duration_ns,
                               total_changes);
  }

  return statistics;
}

}  // namespace hs::session
