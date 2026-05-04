#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <ui/godot/module/game/ruleset_object.hpp>
#include <ui/godot/module/scope/scope_object.hpp>
#include <ui/godot/module/session/session.hpp>
#include <ui/godot/module/terra/world_object.hpp>

namespace hs::godot {

using namespace ::godot;

class SessionObject : public RefCounted {
  GDCLASS(SessionObject, RefCounted);

 public:
  SessionObject() = default;

  void _init() {}

  static void _bind_methods();

  bool set_rule_set(const Ref<RulesetObject>& ruleset);
  bool set_world(const Ref<WorldObject>& world);
  bool add_scope(const Ref<ScopeObject>& scope);
  void advance_next_turn();

 private:
  Session data_;
};

}  // namespace hs::godot
