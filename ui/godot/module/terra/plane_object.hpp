#pragma once

#include <memory>

#include <core/terra/plane.hpp>
#include <ui/godot/module/region/region_object.hpp>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/image.hpp>

using namespace godot;

class PlaneObject : public RefCounted {
  GDCLASS(PlaneObject, RefCounted);

public:
  PlaneObject() {}
  PlaneObject(hs::terra::Plane&& data):
    data_(std::move(data)) {}

  using QRSCoordinateSystem = hs::terra::Plane::QRSCoordinateSystem;
  using QRSCoords = hs::terra::Plane::QRSCoords;

  void _init() {
  }

  //void set_world(std::shared_ptr<terra::Plane> world_ptr);

  static void _bind_methods();


private:
  hs::terra::Plane data_;

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
  static Ref<PlaneObject> create_world(Vector2i world_size, int region_radius);

  static QRSCoords cast_qrs(Vector2i coords) {
    return QRSCoords{
      QRSCoordinateSystem::QAxis{coords.x},
      QRSCoordinateSystem::RAxis{coords.y}
    };
  }
  static auto cast_qrs_size(Vector2i size) {
    auto q_size = typename hs::terra::Plane::QRSCoordinateSystem::QDelta{size.x};
    auto r_size = typename hs::terra::Plane::QRSCoordinateSystem::RDelta{size.y};

    return typename hs::terra::Plane::QRSSize{q_size, r_size};
  }

};



