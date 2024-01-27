#include "system.hpp"

Ref<WorldObject> System::load_world(String filename) {

  Ref<WorldObject> result;
  //result.instantiate();
  result.reference_ptr(new WorldObject(system_->LoadWorld(filename.utf8().get_data())));
  //ERR_FAIL_NULL_V(result.ptr(), result);
  return result;

}

void System::_bind_methods() {
  ClassDB::bind_method(D_METHOD("load_world", "filename"), &System::load_world);
}
