#include "world_object.hpp"

#include <fstream>

#include <ui/godot/module/utils/to_string.hpp>

void WorldObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("save", "filename"), &WorldObject::save);
  ClassDB::bind_method(D_METHOD("load", "filename"), &WorldObject::load);
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
    data_ = ParseFrom(proto_world, ::hs::serialize::To<hs::terra::World>{});
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


