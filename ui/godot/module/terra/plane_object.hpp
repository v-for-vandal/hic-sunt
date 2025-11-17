#pragma once

#include <core/terra/plane.hpp>
#include <core/terra/world.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <memory>
#include <ui/godot/module/region/region_object.hpp>
#include <ui/godot/module/terra/plane.hpp>

namespace hs::godot {

using namespace ::godot;

class PlaneObject : public RefCounted {
  GDCLASS(PlaneObject, RefCounted);

 public:
  PlaneObject() {}
  PlaneObject(::hs::godot::PlanePtr data) : data_(std::move(data)) {}

  using QRSCoordinateSystem = World::QRSCoordinateSystem;
  using QRSCoords = World::QRSCoords;
  using QRSSize = World::QRSSize;
  using Cell = ::hs::godot::Plane::Cell;

  void _init() {}

  static void _bind_methods();

 private:
  PlanePtr data_;

 public:
  StringName get_id() const;
  Rect2i get_dimensions() const;
  Ref<RegionObject> get_region(Vector2i coords) const;
  Ref<RegionObject> get_region_by_id(String region_id) const;
  int get_region_external_radius() const;
  Dictionary get_region_info(Vector2i coords) const;
  void foreach_surface(const Callable& callback);
  bool contains(Vector2i coords) const;
  static Dictionary create_error(const char* error);
  static Dictionary create_success();
  float get_distance_between_cells(Vector2i region1, Vector2i cell1,
                                   Vector2i region2, Vector2i cell2);
  // String get_cell_terrain(Vector2i coords) const;
};

}  // namespace hs::godot
