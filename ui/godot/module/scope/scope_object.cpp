#include "scope_object.hpp"

#include <core/utils/overloaded.hpp>

#define ERR_FAIL_NULL_SCOPE(result) \
  ERR_FAIL_NULL_V_MSG(scope_, result, ERR_MSG_SCOPE_IS_NULL)

namespace hs::godot {

static constexpr const char* ERR_MSG_SCOPE_IS_NULL =
    "null-containing scope object";

void ScopeObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_numeric_value"),
                       &ScopeObject::get_numeric_value);
  ClassDB::bind_method(D_METHOD("get_string_value"),
                       &ScopeObject::get_string_value);
  ClassDB::bind_method(
      D_METHOD("add_numeric_modifier", "variable", "key", "add", "mult"),
      &ScopeObject::add_numeric_modifier);
  ClassDB::bind_method(
      D_METHOD("add_string_modifier", "variable", "key", "value", "level"),
      &ScopeObject::add_string_modifier);
  ClassDB::bind_method(D_METHOD("explain_all"), &ScopeObject::explain_all);
  ClassDB::bind_method(D_METHOD("is_string_variable"), &ScopeObject::is_string_variable);
  ClassDB::bind_static_method("ScopeObject", D_METHOD("create_scope"),
      &ScopeObject::create_scope);
}

float ScopeObject::get_numeric_value(const StringName& variable) {
  ERR_FAIL_NULL_SCOPE(0.0);
  return scope_->GetNumericValue(variable);
}

StringName ScopeObject::get_string_value(const StringName& variable) {
  ERR_FAIL_NULL_SCOPE(StringName{});
  return scope_->GetStringValue(variable);
}

bool ScopeObject::add_numeric_modifier(const StringName& variable,
                                       const StringName& key, float add,
                                       float mult) {
  ERR_FAIL_NULL_SCOPE(false);

  return scope_->AddNumericModifier(variable, key, add, mult);
}

bool ScopeObject::add_string_modifier(const StringName& variable,
                                      const StringName& key,
                                      const StringName& value, float level) {
  ERR_FAIL_NULL_SCOPE(false);

  return scope_->AddStringModifier(variable, key, value, level);
}

bool ScopeObject::is_string_variable(const StringName& variable) const {
  ERR_FAIL_NULL_SCOPE(false);

  return scope_->IsStringVariable(variable);
}

Dictionary ScopeObject::explain_all() {
  Dictionary result;
  ERR_FAIL_NULL_SCOPE(result);

  auto add_numeric_element_fn =
      [&result](StringName scope_id, StringName variable, StringName modifier,
                float add, float mult) {
        Dictionary leaf;
        leaf[StringName{"add"}] = add;
        leaf[StringName{"mult"}] = mult;
        static_cast<Dictionary>(
            static_cast<Dictionary>(result.get_or_add(variable, Dictionary{}))
                .get_or_add(scope_id, Dictionary{}))
            .set(modifier, leaf);
      };

  auto add_string_element_fn =
      [&result](StringName scope_id, StringName variable, StringName modifier,
                StringName value, float level) {
        Dictionary leaf;
        leaf[StringName{"value"}] = value;
        leaf[StringName{"level"}] = level;
        static_cast<Dictionary>(
            static_cast<Dictionary>(result.get_or_add(variable, Dictionary{}))
                .get_or_add(scope_id, Dictionary{}))
            .set(modifier, leaf);
      };

  scope_->ExplainAllVariables(
      hs::utils::Overloaded{add_numeric_element_fn, add_string_element_fn});
  return result;
}

Ref<ScopeObject> ScopeObject::create_scope() {
    auto scope_ptr = ScopePtr{}; // non-null ptr will create new object
    Ref<ScopeObject> result(memnew(ScopeObject(scope_ptr)));
    ERR_FAIL_NULL_V_MSG(result.ptr(), result, "Failed to create scope object");
    return result;
}


}  // namespace hs::godot
