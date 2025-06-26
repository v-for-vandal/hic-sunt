#pragma once

#include <memory>

#include <core/terra/plane.hpp>
#include <core/terra/world.hpp>
#include <ui/godot/module/region/region_object.hpp>
#include <ui/godot/module/terra/plane.hpp>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/callable.hpp>


namespace hs::godot {

using namespace ::godot;

class PlaneObject : public RefCounted {
  GDCLASS(PlaneObject, RefCounted);

public:
  PlaneObject() {}
  PlaneObject(::hs::godot::PlanePtr data):
    data_(std::move(data)) {}

  using QRSCoordinateSystem = World::QRSCoordinateSystem;
  using QRSCoords = World::QRSCoords;
  using QRSSize = World::QRSSize;
  using Cell = ::hs::godot::Plane::Cell;

  void _init() {
  }

  static void _bind_methods();


private:
  PlanePtr data_;

public:
  Rect2i get_dimensions() const;
  Ref<RegionObject> get_region(Vector2i coords) const;
  Ref<RegionObject> get_region_by_id(String region_id) const;
  Dictionary get_region_info(Vector2i coords) const;
  void foreach_surface(const Callable& callback);
  bool contains(Vector2i coords) const;
  static Dictionary create_error(const char* error);
  static Dictionary create_success();
  //String get_cell_terrain(Vector2i coords) const;

};


}
