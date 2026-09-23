#pragma once

#include <absl/container/flat_hash_map.h>
#include <session/session.pb.h>

#include <core/ruleset/ruleset.hpp>
#include <core/scope/scope.hpp>
#include <core/terra/types.hpp>
#include <core/terra/world.hpp>
#include <core/types/error_code.hpp>
#include <core/types/scope_type.hpp>
#include <core/utils/serialize.hpp>
#include <expected>
#include <filesystem>
#include <memory>
#include <vector>

#include "effect_instance.hpp"

namespace hs::session {

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
class Session;

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
void SerializeTo(const Session<BaseTypes, WorldPtr, RuleSetPtr>& source,
                 proto::session::Session& target);

template <typename BaseTypes, typename WorldPtr, typename RuleSetPtr>
Session<BaseTypes, WorldPtr, RuleSetPtr> ParseFrom(
    const proto::session::Session& source, serialize::To<Session<BaseTypes, WorldPtr, RuleSetPtr>>);

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
  using RuleSet = ruleset::RuleSet<BaseTypes>;
  using CivilizationPtr = terra::CivilizationPtr<BaseTypes>;

  Session() = default;

  std::expected<void, ErrorCode> SetRuleSet(RuleSetPtr ptr);

  std::expected<void, ErrorCode> SetWorld(WorldPtr ptr);
  const auto& GetWorld() const noexcept { return world_; }

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

  // This function creates new improvement scope (and also all realated class and tag
  // scopes) It does not place the improvement on the map - it only creates scope.
  // Created scope is orphaned. After placing it on the map, it must be registered with session
  std::expected<ScopePtr, ErrorCode> CreateImprovementScope(StringId civ_id,
                                                            StringId improvement_class);

  // This function creates and registers new civilization with given id
  std::expected<CivilizationPtr, ErrorCode> CreateCivilization(StringId civ_id);

  // This function creates new city scope and registers it with session
  std::expected<ScopePtr, ErrorCode> CreateCity(StringId civ_id);

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
  [[nodiscard]] bool ValidateCoreVariablesInRuleSet(const RuleSet& ruleset) const;
  void Prepare();

 private:
  friend class EffectExecutor<BaseTypes>;
  friend void SerializeTo<BaseTypes, WorldPtr, RuleSetPtr>(const Session& source,
                                                           proto::session::Session& target);
  friend Session ParseFrom<BaseTypes, WorldPtr, RuleSetPtr>(const proto::session::Session& source,
                                                            serialize::To<Session>);

  // This function will create improvement class scope and propertly initialize it
  std::expected<ScopePtr, ErrorCode> CreateImprovementClassScope(const CivilizationPtr& civ,
                                                                 StringId improvement_class);

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
