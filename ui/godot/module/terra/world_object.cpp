#include "world_object.hpp"

#include <fstream>

#include <ui/godot/module/utils/to_string.hpp>
#include <ui/godot/module/utils/cast_qrs.hpp>

namespace hs::godot {

void WorldObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("save", "filename"), &WorldObject::save);
  ClassDB::bind_method(D_METHOD("load", "filename"), &WorldObject::load);
  ClassDB::bind_method(D_METHOD("create_plane", "plane_id", "box", "region_radius"), &WorldObject::create_plane);
  ClassDB::bind_method(D_METHOD("get_plane", "plane_id"), &WorldObject::get_plane);
  ClassDB::bind_static_method("WorldObject", D_METHOD("create_world"), &WorldObject::create_world);
}


Error WorldObject::save(String filename) {
  hs::proto::terra::World proto_world;
  SerializeTo(data_, proto_world);

  try {
    std::fstream output(filename.utf8().get_data(), std::ios::out | std::ios::trunc | std::ios::binary);
    if (!proto_world.SerializeToOstream(&output)) {
      spdlog::error("Can't write filename {} as proto object", filename.utf8().get_data());
      return ERR_FILE_CANT_WRITE;
    }
  } catch(const std::exception& e) {
    return ERR_FILE_CANT_OPEN;
  }
  return OK;
}

Error WorldObject::load(String filename) {
  hs::proto::terra::World proto_world;

  try {
    std::fstream input(filename.utf8().get_data(), std::ios::in | std::ios::binary);
    if (!proto_world.ParseFromIstream(&input)) {
      spdlog::error("Can't read filename {} as proto object", filename.utf8().get_data());
      return ERR_FILE_CANT_READ;
    }
  } catch(const std::exception& e) {
    return ERR_FILE_CANT_OPEN;
  }
  try {
    data_ = ParseFrom(proto_world, ::hs::serialize::To<World>{});
  } catch (const std::exception&e) {
    return ERR_FILE_CORRUPT;
  }


  return OK;

}

Dictionary WorldObject::create_error(const char* error) {
    Dictionary result;
    result["success"] = false;
    result["error"] = error;
    return result;

}

Dictionary WorldObject::create_success() {
    Dictionary result;
    result["success"] = true;
    return result;
}

Ref<WorldObject> WorldObject::create_world() {

  Ref<WorldObject> result(memnew(WorldObject()));

  ERR_FAIL_NULL_V_MSG(result.ptr(), result, "Failed to create new world");
  return result;

}

Ref<PlaneObject> WorldObject::get_plane(StringName name) {
  auto plane_ptr = data_.GetPlane(name);
  if (!plane_ptr) {
    return Ref<PlaneObject>();
  }

  Ref<PlaneObject> result(memnew(PlaneObject(
        plane_ptr)));
  ERR_FAIL_NULL_V_MSG(result.ptr(), result, "Failed to get plane");
  return result;
}

Ref<PlaneObject> WorldObject::create_plane(StringName name, Rect2i box, int region_radius) {
  auto plane_ptr = data_.AddPlane(name, cast_qrs_box(box), region_radius);
  Ref<PlaneObject> result(memnew(PlaneObject(
        plane_ptr)));
  ERR_FAIL_NULL_V_MSG(result.ptr(), result, "Failed to create new plane");
  return result;
}

}
