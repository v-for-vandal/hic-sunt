#pragma once

#include <memory>

#include <core/terra/world.hpp>
#include <ui/godot/module/region/region_object.hpp>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/image.hpp>

using namespace godot;

class WorldObject : public RefCounted {
  GDCLASS(WorldObject, RefCounted);

public:
  WorldObject() {}
  WorldObject(hs::terra::World&& data):
    data_(std::move(data)) {}

  using QRSCoordinateSystem = hs::terra::World::QRSCoordinateSystem;
  using QRSCoords = hs::terra::World::QRSCoords;

  void _init() {
  }

  static void _bind_methods();


private:
  hs::terra::World data_;

public:
  Error save(String filename);
  Error load(String filename);
  static Dictionary create_error(const char* error);
  static Dictionary create_success();
  static Ref<WorldObject> create_world();

  static QRSCoords cast_qrs(Vector2i coords) {
    return QRSCoords{
      QRSCoordinateSystem::QAxis{coords.x},
      QRSCoordinateSystem::RAxis{coords.y}
    };
  }
  static auto cast_qrs_size(Vector2i size) {
    auto q_size = typename hs::terra::World::QRSCoordinateSystem::QDelta{size.x};
    auto r_size = typename hs::terra::World::QRSCoordinateSystem::RDelta{size.y};

    return typename hs::terra::World::QRSSize{q_size, r_size};
  }

};


