#include "session_object.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>

namespace hs::godot {

namespace {

Dictionary MakeEffectExecutionStatisticsDictionary(
    const hs::session::EffectExecutionStatistics<GodotBaseTypes>& statistics) {
  Dictionary result;

  for (const auto& [effect_id, entry] : statistics.GetEntries()) {
    Dictionary item;
    item["execution_count"] = static_cast<int64_t>(entry.execution_count);
    item["total_duration_ns"] = static_cast<int64_t>(entry.total_duration_ns);
    item["total_changes"] = static_cast<int64_t>(entry.total_changes);
    item["failure_count"] = static_cast<int64_t>(entry.failure_count);
    item["partial_failure_count"] = static_cast<int64_t>(entry.partial_failure_count);
    result[effect_id] = item;
  }

  return result;
}

}  // namespace

void SessionObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_ruleset", "ruleset"), &SessionObject::set_ruleset);
  ClassDB::bind_method(D_METHOD("set_world", "world"), &SessionObject::set_world);
  ClassDB::bind_method(D_METHOD("add_scope", "scope"), &SessionObject::add_scope);
  ClassDB::bind_method(D_METHOD("advance_next_turn"), &SessionObject::advance_next_turn);
  ClassDB::bind_method(D_METHOD("set_current_turn"), &SessionObject::set_current_turn);
  ClassDB::bind_method(D_METHOD("get_current_turn"), &SessionObject::get_current_turn);
  ClassDB::bind_method(D_METHOD("get_last_effect_execution_statistics"),
                       &SessionObject::get_last_effect_execution_statistics);
  ClassDB::bind_method(D_METHOD("get_total_effect_execution_statistics"),
                       &SessionObject::get_total_effect_execution_statistics);
}

bool SessionObject::set_ruleset(const Ref<RulesetObject>& ruleset) {
  ERR_FAIL_NULL_V_MSG(ruleset.ptr(), false, "null-containing ruleset object");

  auto result = data_.SetRuleSet(RuleSetPtrAdapter(ruleset));
  ERR_FAIL_COND_V_MSG(!result.has_value(), false, "Failed to set ruleset on session");
  return true;
}

bool SessionObject::set_world(const Ref<WorldObject>& world) {
  ERR_FAIL_NULL_V_MSG(world.ptr(), false, "null-containing world object");

  auto result = data_.SetWorld(WorldPtrAdapter(world));
  ERR_FAIL_COND_V_MSG(!result.has_value(), false, "Failed to set world on session");
  return true;
}

bool SessionObject::add_scope(const Ref<ScopeObject>& scope) {
  ERR_FAIL_NULL_V_MSG(scope.ptr(), false, "null-containing scope object");

  const auto& scope_ptr = scope->GetScopePtr();
  ERR_FAIL_COND_V_MSG(!scope_ptr, false, "scope object does not contain scope");

  auto result = data_.AddScope(scope_ptr);
  ERR_FAIL_COND_V_MSG(!result.has_value(), false, "Failed to add scope to session");
  return true;
}

void SessionObject::set_current_turn(int turn) { data_.SetCurrentTurn(turn); }

int SessionObject::get_current_turn() const { return data_.GetCurrentTurn(); }

void SessionObject::advance_next_turn() { data_.AdvanceNextTurn(); }

Dictionary SessionObject::get_last_effect_execution_statistics() const {
  return MakeEffectExecutionStatisticsDictionary(data_.GetLastEffectExecutionStatistics());
}

Dictionary SessionObject::get_total_effect_execution_statistics() const {
  return MakeEffectExecutionStatisticsDictionary(data_.GetTotalEffectExecutionStatistics());
}

}  // namespace hs::godot
