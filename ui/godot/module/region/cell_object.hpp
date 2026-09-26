#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>
#include <memory>
#include <ui/godot/module/game/ruleset_object.hpp>
#include <ui/godot/module/region/region.hpp>
#include <ui/godot/module/scope/scope.hpp>
#include <ui/godot/module/scope/scope_mixin.hpp>
#include <ui/godot/module/scope/scope_object.hpp>
#include <ui/godot/module/terra/world.hpp>
#include <utility>

#include "cell.hpp"

namespace hs::godot {

using namespace ::godot;

class RegionObject;  // forward declaration to avoid circular include

class CellObject : public RefCounted, public ScopeMixin {
  GDCLASS(CellObject, RefCounted);

 public:
  using QRSCoordinateSystem = World::QRSCoordinateSystem;
  using QRSCoords = World::QRSCoords;

  CellObject();
  CellObject(std::shared_ptr<Region> region, QRSCoords cell_coords);

  void _init() {}

  static void _bind_methods();

  ScopePtr GetScope() const;
  Cell& GetCell();
  auto&& GetRegion(this auto&& self) {
    // std::forward maintains the exact const and reference type of 'self'
    return std::forward_like<decltype(self)>(self.region_);
  }
  StringName GetId() const { return GetScope()->GetId(); }

  bool IsValid() const noexcept {
    if (region_ == nullptr) [[unlikely]] {
      return false;
    }

    if (!region_->GetSurface().Contains(cell_coords_)) [[unlikely]] {
      return false;
    }

    return true;
  }

  bool is_valid() const noexcept { return IsValid(); }
  Ref<ScopeObject> get_improvement(int slot);
  TypedDictionary<int, ScopeObject> get_improvements();
  Ref<ScopeObject> get_scope() { return ScopeMixin::get_scope(); }
  Ref<RegionObject> get_region();
  StringName get_region_id() const;
  StringName get_id() const { return GetScope()->GetId(); }

 private:
  // Unlike other Godot objects, here we store pointer to region
  // and qrs coords of a cell
  std::shared_ptr<Region> region_;
  QRSCoords cell_coords_;
  Ref<RegionObject> region_object_;

  static ScopePtr CreateInvalidCellScope();
};

}  // namespace hs::godot
