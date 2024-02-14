#pragma once

#include <memory>

#include <core/terra/world.hpp>

#include <godot_cpp/classes/ref.hpp>

using namespace godot;

class RegionObject : public RefCounted {
  GDCLASS(RegionObject, RefCounted);

public:
  RegionObject() {}
  RegionObject(std::shared_ptr<hs::region::Region> region):
    region_(std::move(region)) {}

  using QRSCoordinateSystem = hs::terra::World::QRSCoordinateSystem;
  using QRSCoords = hs::terra::World::QRSCoords;

  void _init() {
  }

  //void set_world(std::shared_ptr<terra::World> world_ptr);

  static void _bind_methods();

  static Dictionary make_region_info(const hs::region::Region& region);


private:
  std::shared_ptr<hs::region::Region> region_; // never null

public:
  Vector2i get_dimensions() const;
  Dictionary get_cell_info(Vector2i coords) const;
  bool set_terrain(Vector2i coords, String terrain) const;
  bool set_feature(Vector2i coords, String feature) const;
  Array get_available_improvements() const;

  //Array get_region_improvements() const;

  static QRSCoords to_qrs(Vector2i coords) {
    return QRSCoords{
      QRSCoordinateSystem::QAxis{coords.x},
      QRSCoordinateSystem::RAxis{coords.y}
    };
  }

};
