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
  Rect2i get_dimensions() const;
  Ref<RegionObject> get_region(Vector2i coords) const;
  Ref<RegionObject> get_region_by_id(String region_id) const;
  Dictionary get_region_info(Vector2i coords) const;
  bool contains(Vector2i coords) const;
  Error save(String filename);
  Error load(String filename);
  static Dictionary create_error(const char* error);
  static Dictionary create_success();
  //String get_cell_terrain(Vector2i coords) const;
  static Ref<WorldObject> create_world(Vector2i world_size, int region_radius);

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


