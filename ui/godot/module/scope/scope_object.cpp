#include "scope_object.hpp"

namespace hs::godot {

void ScopeObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_numeric_value"), &ScopeObject::get_numeric_value);
  ClassDB::bind_method(D_METHOD("get_string_value"), &ScopeObject::get_string_value);
  ClassDB::bind_method(D_METHOD("add_numeric_modifier", "variable", "key", "add", "mult"),
      &ScopeObject::add_numeric_modifier);
}

float ScopeObject::get_numeric_value(const StringName& variable)
{
    ERR_FAIL_NULL_V_MSG(scope_, 0.0, "Scope is not present");
    return scope_->GetNumericValue(variable);
}


String ScopeObject::get_string_value(const StringName& variable)
{
    ERR_FAIL_NULL_V_MSG(scope_, String{}, "Scope is not present");
    return scope_->GetStringValue(variable);
}


bool ScopeObject::add_numeric_modifier(const StringName& variable, const StringName& key,
  float add, float mult)
{
    ERR_FAIL_NULL_V_MSG(scope_, false, "Scope is not present");

    scope_->AddNumericModifier(variable, key, add, mult);
    return true;
}


}
