#pragma once

#include <memory>

#include <ui/godot/module/scope/scope.hpp>

#include <godot_cpp/classes/ref.hpp>

namespace hs::godot {

using namespace ::godot;

class ScopeObject : public RefCounted {
  GDCLASS(ScopeObject, RefCounted);

public:
  ScopeObject() {}
  ScopeObject(const ScopePtr& scope):
    scope_(std::move(scope)) {}

  static void _bind_methods();

public:
  float get_numeric_value(const StringName& variable);
  StringName get_string_value(const StringName& variable);

  bool add_numeric_modifier(const StringName& variable, const StringName& key,
      float add, float mult);
    bool add_string_modifier(const StringName& variable, const StringName& key,
  const StringName& value, float level);

  Dictionary explain_all();

private:
  ScopePtr scope_;

};


}
