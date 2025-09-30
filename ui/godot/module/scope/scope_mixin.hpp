#pragma once

#include <memory>

#include <ui/godot/module/scope/scope.hpp>

#include <godot_cpp/classes/ref.hpp>

namespace hs::godot {

using namespace ::godot;

/** \brief This class can be mixed in into any class that has GetScope() method
 *         and it will provide godot bindings
 */
class ScopeMixin {

public:
  ScopeMixin() {}

  template<typename T>
  static void _bind_methods() {
    ClassDB::bind_method(D_METHOD("get_scope"), &T::get_scope);
  }

  Ref<ScopeObject> get_scope(this auto& self) {
    auto scope_ptr = self.GetScope();
    ERR_FAIL_NULL_V_MSG(scope_ptr.ptr(), Ref<ScopeObject>{}, "Failed to get scope object");

    Ref<ScopeObject> result(memnew(ScopeObject(scope_)));
    ERR_FAIL_NULL_V_MSG(result.ptr(), result, "Failed to get scope object");
    return result;
  }

};


}

