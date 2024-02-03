#include "system.hpp"

Ref<WorldObject> HicSuntSystem::load_world(String filename) {

  Ref<WorldObject> result;
  result.reference_ptr(new WorldObject(system_->LoadWorld(filename.utf8().get_data())));
  //ERR_FAIL_NULL_V(result.ptr(), result);
  return result;

}

Ref<WorldObject> HicSuntSystem::create_world(Vector2i size) {
  hs::system::NewWorldParameters params;
  auto q_size = typename hs::terra::World::QRSCoordinateSystem::QDelta{size.x};
  auto r_size = typename hs::terra::World::QRSCoordinateSystem::RDelta{size.y};

  params.world_size = typename hs::terra::World::QRSSize{q_size, r_size};
  Ref<WorldObject> result(memnew(WorldObject(system_->NewWorld(params))));
  //result.reference_ptr());
  ERR_FAIL_NULL_V_MSG(result.ptr(), result, "Failed to create new world");
  return result;

}

Dictionary HicSuntSystem::load_ruleset(String folder_path) {
  hs::utils::ErrorsCollection errors;
  const bool success = system_->LoadRuleSet(folder_path.utf8().get_data(), errors);
  Dictionary result;
  Array errors_godot;
  for(auto& error_msg: errors.errors) {
    errors_godot.append(String(error_msg.message.c_str()));
  }
  result[String("errors")] = errors_godot;
  result[String("success")] = success;

  return result;

}

void HicSuntSystem::_bind_methods() {
  ClassDB::bind_method(D_METHOD("load_world", "filename"), &HicSuntSystem::load_world);
  ClassDB::bind_method(D_METHOD("create_world", "size"), &HicSuntSystem::create_world);
  ClassDB::bind_method(D_METHOD("load_ruleset", "folder_path"), &HicSuntSystem::load_ruleset);
}
