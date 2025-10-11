#pragma once

#include <memory>

#include <ui/godot/module/region/region.hpp>
#include <ui/godot/module/scope/scope_mixin.hpp>
#include <ui/godot/module/scope/scope_object.hpp>
#include <ui/godot/module/terra/world.hpp>
#include <ui/godot/module/game/ruleset_object.hpp>

#include <godot_cpp/classes/ref.hpp>


namespace hs::godot {

using namespace ::godot;


class CellObject : public RefCounted, public ScopeMixin  {
  GDCLASS(CellObject, RefCounted);

public:
  using QRSCoordinateSystem = World::QRSCoordinateSystem;
  using QRSCoords = World::QRSCoords;


  CellObject() {}
  CellObject(std::shared_ptr<Region> region, QRSCoords cell_coords):
    region_(std::move(region)),
    cell_coords_(cell_coords) {}


  void _init() {
  }

  static void _bind_methods();

  ScopePtr GetScope() const;

private:
  // Unlike other Godot objects, here we store pointer to region
  // and qrs coords of a cell
  std::shared_ptr<Region> region_;
  QRSCoords cell_coords_;

public:

  Ref<ScopeObject> get_scope() { return ScopeMixin::get_scope(); }
  String get_region_id() const;

};

}

