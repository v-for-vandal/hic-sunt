#include "session_object.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>

namespace hs::godot {

void SessionObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_rule_set", "ruleset"),
                       &SessionObject::set_rule_set);
  ClassDB::bind_method(D_METHOD("set_world", "world"),
                       &SessionObject::set_world);
  ClassDB::bind_method(D_METHOD("add_scope", "scope"),
                       &SessionObject::add_scope);
  ClassDB::bind_method(D_METHOD("advance_next_turn"),
                       &SessionObject::advance_next_turn);
}

bool SessionObject::set_rule_set(const Ref<RulesetObject>& ruleset) {
  ERR_FAIL_NULL_V_MSG(ruleset.ptr(), false, "null-containing ruleset object");

  auto result = data_.SetRuleSet(RuleSetPtrAdapter(ruleset));
  ERR_FAIL_COND_V_MSG(!result.has_value(), false,
                      "Failed to set ruleset on session");
  return true;
}

bool SessionObject::set_world(const Ref<WorldObject>& world) {
  ERR_FAIL_NULL_V_MSG(world.ptr(), false, "null-containing world object");

  auto result = data_.SetWorld(WorldPtrAdapter(world));
  ERR_FAIL_COND_V_MSG(!result.has_value(), false,
                      "Failed to set world on session");
  return true;
}

bool SessionObject::add_scope(const Ref<ScopeObject>& scope) {
  ERR_FAIL_NULL_V_MSG(scope.ptr(), false, "null-containing scope object");

  const auto& scope_ptr = scope->GetScopePtr();
  ERR_FAIL_COND_V_MSG(!scope_ptr, false,
                      "scope object does not contain scope");

  auto result = data_.AddScope(scope_ptr);
  ERR_FAIL_COND_V_MSG(!result.has_value(), false,
                      "Failed to add scope to session");
  return true;
}

void SessionObject::advance_next_turn() { data_.AdvanceNextTurn(); }

}  // namespace hs::godot
