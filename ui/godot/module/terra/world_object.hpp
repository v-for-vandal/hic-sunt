#pragma once

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <memory>
#include <ui/godot/module/region/region_object.hpp>
#include <ui/godot/module/terra/plane_object.hpp>
#include <ui/godot/module/terra/world.hpp>

namespace hs::godot {

using namespace godot;

class WorldObject : public RefCounted {
  GDCLASS(WorldObject, RefCounted);

 public:
  WorldObject() {}
  WorldObject(hs::godot::World&& data) : data_(std::move(data)) {}

  using QRSCoordinateSystem = World::QRSCoordinateSystem;
  using QRSCoords = World::QRSCoords;

  void _init() {}

  static void _bind_methods();

 private:
  World data_;

 public:
  Error save(String filename);
  Error load(String filename);
  Ref<PlaneObject> create_plane(StringName plane_id, Rect2i box,
                                int region_radius, int region_external_radius);
  Ref<PlaneObject> get_plane(StringName plane_id);
  Ref<RegionObject> get_region_by_id(StringName region_id);
  static Dictionary create_error(const char* error);
  static Dictionary create_success();
  static Ref<WorldObject> create_world();
};

}  // namespace hs::godot
