#pragma once

#include <memory>

#include <core/terra/world.hpp>
#include <ui/godot/module/region/region_object.hpp>

#include <godot_cpp/classes/ref.hpp>

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

  //void set_world(std::shared_ptr<terra::World> world_ptr);

  static void _bind_methods();


private:
  hs::terra::World data_;

public:
  Vector2i get_dimensions() const;
  Ref<RegionObject> get_region(Vector2i coords) const;
  Dictionary get_region_info(Vector2i coords) const;
  //String get_cell_terrain(Vector2i coords) const;

  // TODO: Move this method to System ?
  Array get_region_improvements() const;

  static QRSCoords to_qrs(Vector2i coords) {
    return QRSCoords{
      QRSCoordinateSystem::QAxis{coords.x},
      QRSCoordinateSystem::RAxis{coords.y}
    };
  }

};


