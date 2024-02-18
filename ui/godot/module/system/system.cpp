#include "system.hpp"


namespace {
  auto to_qrs_size(Vector2i size) {
    auto q_size = typename hs::terra::World::QRSCoordinateSystem::QDelta{size.x};
    auto r_size = typename hs::terra::World::QRSCoordinateSystem::RDelta{size.y};

    return typename hs::terra::World::QRSSize{q_size, r_size};
  }

}

Ref<WorldObject> HicSuntSystem::load_world(String filename) {

  Ref<WorldObject> result;
  //result.reference_ptr(new WorldObject(system_->LoadWorld(filename.utf8().get_data())));
  //ERR_FAIL_NULL_V(result.ptr(), result);
  return result;

}

Ref<WorldObject> HicSuntSystem::create_world(Vector2i world_size, int region_radius,
  RulesetObject* ruleset) {

  ERR_FAIL_NULL_V_MSG(ruleset, Ref<WorldObject>{}, "Nullptr as ruleset when creating world");

  hs::system::NewWorldParameters params;

  params.world_size = to_qrs_size(world_size);
  params.region_size = region_radius;
  Ref<WorldObject> result(memnew(WorldObject(system_->NewWorld(params, ruleset->GetRuleSet()))));
  //result.reference_ptr());
  ERR_FAIL_NULL_V_MSG(result.ptr(), result, "Failed to create new world");
  return result;

}

Dictionary HicSuntSystem::load_ruleset(String folder_path) {
  Dictionary result;

  hs::utils::ErrorsCollection errors;
  auto ruleset_opt = system_->LoadRuleSet(folder_path.utf8().get_data(), errors);

  Array errors_godot;
  for(auto& error_msg: errors.errors) {
    errors_godot.append(String(error_msg.message.c_str()));
  }
  result[String("errors")] = errors_godot;

  if(ruleset_opt.has_value()) {
    Ref<RulesetObject> ruleset(memnew(RulesetObject(std::move(ruleset_opt.value()))));
    result[String("ruleset")] = ruleset;
    result[String("success")] = true;
  } else {
    result[String("success")] = false;
  }

  return result;

}

void HicSuntSystem::_bind_methods() {
  ClassDB::bind_method(D_METHOD("load_world", "filename"), &HicSuntSystem::load_world);
  ClassDB::bind_method(D_METHOD("create_world", "world_size", "region_size", "ruleset"), &HicSuntSystem::create_world);
  ClassDB::bind_method(D_METHOD("load_ruleset", "folder_path"), &HicSuntSystem::load_ruleset);
}
