#include "system.hpp"

Ref<WorldObject> HicSuntSystem::load_world(String filename) {

  Ref<WorldObject> result;
  result.reference_ptr(new WorldObject(system_->LoadWorld(filename.utf8().get_data())));
  //ERR_FAIL_NULL_V(result.ptr(), result);
  return result;

}

Ref<WorldObject> HicSuntSystem::create_world(Vector2i size) {
  hs::system::NewWorldParameters params;
  params.q_size = typename hs::terra::World::QRSCoordinateSystem::QDelta{size.x};
  params.r_size = typename hs::terra::World::QRSCoordinateSystem::RDelta{size.y};
  Ref<WorldObject> result(memnew(WorldObject(system_->NewWorld(params))));
  //result.reference_ptr());
  ERR_FAIL_NULL_V_MSG(result.ptr(), result, "Failed to create new world");
  return result;

}

void HicSuntSystem::_bind_methods() {
  ClassDB::bind_method(D_METHOD("load_world", "filename"), &HicSuntSystem::load_world);
  ClassDB::bind_method(D_METHOD("create_world", "size"), &HicSuntSystem::create_world);
}
