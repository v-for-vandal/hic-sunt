#include "world_object.hpp"

void WorldObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_dimensions"), &WorldObject::get_dimensions);
  ClassDB::bind_method(D_METHOD("get_cell_terrain", "coords"), &WorldObject::get_cell_terrain);
  ClassDB::bind_method(D_METHOD("get_region_improvements"), &WorldObject::get_region_improvements);
}

Vector2i WorldObject::get_dimensions() const {
  return Vector2i{
    data_.GetSurface().q_size().ToUnderlying(),
    data_.GetSurface().r_size().ToUnderlying()
  };
}

String WorldObject::get_cell_terrain(Vector2i coords) const {
  return "plains";
  // TODO: FIX
  /*
  auto qrs_coords = to_qrs(coords);

  if (!data_.GetSurface().Contains(qrs_coords)) {
    return {};
  }

  return data_.GetSurface().GetCell(qrs_coords).GetTerrain().data();
  */
}

Array WorldObject::get_region_improvements() const {
  const auto& ruleset = data_.GetRuleSet();

  const auto& improvements = ruleset.GetRegionImprovements();

  Array result;

  for(auto& improvement: improvements.improvements()) {
    Dictionary godot_improvement_info;
    godot_improvement_info["id"] = improvement.id().c_str();

    result.append(std::move(godot_improvement_info));
  }

  return result;
}

#if 0

void WorldObject::set_world(std::shared_ptr<terra::World> world_ptr) {
  /*
  world_ptr_ = std::move(world_ptr);
  start_point_ = Vector3FromCoords(world_ptr_->begin_point());
  end_point_ = Vector3FromCoords(world_ptr_->end_point());
  mc_start_point_ = Vector3FromCoords(world_ptr_->GetMainCellsView().begin_point());
  mc_end_point_ = Vector3FromCoords(world_ptr_->GetMainCellsView().end_point());
  */
}

Ref<CellObject> WorldObject::GetCellObject(Vector3 godot_coords) const {
  using RawCoordinateSystem = terra::RawCoordinateSystem;

  Ref<CellObject> result;
  auto coords = CoordsFromVector3<RawCoordinateSystem>(godot_coords);
  ERR_FAIL_NULL_V(world_ptr_, result);
  ERR_FAIL_COND_V(!world_ptr_->Contains(coords), result);

  result.instance();
  ERR_FAIL_NULL_V(result.ptr(), result);

  const auto& cell = world_ptr_->GetCell(coords);
  result->init(world_ptr_, cell);

  return result;

}
#endif


