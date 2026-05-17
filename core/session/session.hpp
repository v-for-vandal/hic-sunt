#pragma once

#include <absl/container/flat_hash_map.h>

#include <core/ruleset/ruleset.hpp>
#include <core/scope/scope.hpp>
#include <core/terra/world.hpp>
#include <core/types/error_code.hpp>
#include <core/types/scope_type.hpp>
#include <expected>
#include <filesystem>
#include <memory>
#include <vector>

#include "effect_instance.hpp"

namespace hs::session {

template <typename BaseTypes>
class EffectExecutor;

template <typename BaseTypes>
class EffectExecutionStatistics;

template <typename BaseTypes = StdBaseTypes,
          typename WorldPtr = std::shared_ptr<terra::World<BaseTypes>>,
          typename RuleSetPtr = std::shared_ptr<ruleset::RuleSet<BaseTypes>>>
class Session {
 public:
  using Scope = scope::Scope<BaseTypes>;
  using ScopePtr = scope::ScopePtr<BaseTypes>;
  using StringId = typename BaseTypes::StringId;
  using ScopeType = types::ScopeType;

  Session() = default;

  std::expected<void, ErrorCode> SetRuleSet(RuleSetPtr ptr);

  std::expected<void, ErrorCode> SetWorld(WorldPtr ptr);

  std::expected<void, ErrorCode> AddScope(const ScopePtr& scope);

  // Advance next turn will move timeline to next turn and execute all
  // required scripts, functions and so on.
  [[nodiscard]] std::expected<void, ErrorCode> AdvanceNextTurn();

  // This function will change internal turn counter without any logic. Its
  // primary use is to set current turn when loading game. Changing turn
  // to earlier value is forbidden because it messes up caches.
  void SetCurrentTurn(size_t value);

  // Return current turn
  size_t GetCurrentTurn() const { return current_turn_; }

  // This function creates and registers new improvement scope (and also all realated class and tag scopes)
  // It does not place the improvement on the map - it only creates scope.
  std::expected<ScopePtr, ErrorCode> CreateImprovementScope(StringId civ_id, StringId improvement_class);

  const auto& GetScopesById() const noexcept { return scopes_by_id_; }
  const auto& GetScopesByType() const noexcept { return scopes_by_type_; }
  auto& GetEffects() noexcept { return effects_; }
  const auto& GetEffects() const noexcept { return effects_; }
  const auto& GetLastEffectExecutionStatistics() const noexcept {
    return last_effect_execution_statistics_;
  }
  const auto& GetTotalEffectExecutionStatistics() const noexcept {
    return total_effect_execution_statistics_;
  }

 private:
  void Prepare();

 private:
  friend class EffectExecutor<BaseTypes>;

  RuleSetPtr ruleset_;
  WorldPtr world_;
  std::size_t current_turn_{0};
  absl::flat_hash_map<StringId, ScopePtr> scopes_by_id_;
  absl::flat_hash_map<ScopeType, std::vector<ScopePtr>> scopes_by_type_;
  std::vector<std::shared_ptr<EffectInstance<BaseTypes>>> effects_;
  EffectExecutionStatistics<BaseTypes> last_effect_execution_statistics_;
  EffectExecutionStatistics<BaseTypes> total_effect_execution_statistics_;

  // static StringId aren't possible because godot::StringName requires godot
  // runtime being active.
  StringId kCoreTurn{"core.turn"};
  StringId kCoreClass{"core.class"};
};

}  // namespace hs::session

#include "session.inl"
