#include "world_object.hpp"

void WorldObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_dimensions"), &WorldObject::get_dimensions);
  ClassDB::bind_method(D_METHOD("get_region", "coords"), &WorldObject::get_region);
  ClassDB::bind_method(D_METHOD("contains", "coords"), &WorldObject::contains);
  ClassDB::bind_method(D_METHOD("get_region_info", "coords"), &WorldObject::get_region_info);
  ClassDB::bind_method(D_METHOD("get_terrain_types"), &WorldObject::get_terrain_types);
  ClassDB::bind_method(D_METHOD("get_region_improvements"), &WorldObject::get_region_improvements);
}

Vector2i WorldObject::get_dimensions() const {
  return Vector2i{
    data_.GetSurface().q_size().ToUnderlying(),
    data_.GetSurface().r_size().ToUnderlying()
  };
}

Ref<RegionObject> WorldObject::get_region(Vector2i coords) const {
  auto qrs_coords = cast_qrs(coords);

  if (!data_.GetSurface().Contains(qrs_coords)) {
    SPDLOG_INFO("no region at coords {},{}", qrs_coords.q(), qrs_coords.r());
    return {};
  }

  Ref<RegionObject> result(memnew(RegionObject(
      data_.GetSurface().GetCell(qrs_coords).GetRegionPtr()
        )));

  return result;
}

Array WorldObject::get_terrain_types() const {
  Array result;

  const auto& ruleset = data_.GetRuleSet();

  const auto& terrain = ruleset.GetTerrain();

  for(const auto& terrain_type : terrain.terrain_types()) {
    result.append(convert_terrain_type(terrain_type));
  }

  return result;
}

Dictionary WorldObject::convert_terrain_type(const hs::proto::ruleset::TerrainType& terrain_type) {
  Dictionary result;
  result["id"] = terrain_type.id().c_str();
  result["render"] = convert_render(terrain_type.render());

  return result;
}

Dictionary WorldObject::convert_render(const hs::proto::render::AtlasRender& render) {
  Dictionary result;
  result["resource"] = render.resource().c_str();
  //render["source_id"] = terrain_type.render().resource();
  {
    int x = 0;
    int y = 0;
    if (render.atlas_coords_size() > 0) {
      x = render.atlas_coords(0);
    }
    if (render.atlas_coords_size() > 0) {
      y = render.atlas_coords(1);
    }
    result["atlas_coords"] = Vector2i(x,y);
  }

  return result;
}

bool WorldObject::contains(Vector2i coords) const {
  auto qrs_coords = cast_qrs(coords);

  return data_.GetSurface().Contains(qrs_coords);
}

Dictionary WorldObject::get_region_info(Vector2i coords) const {
  auto qrs_coords = cast_qrs(coords);

  if (!data_.GetSurface().Contains(qrs_coords)) {
    SPDLOG_INFO("no region at coords {},{}", qrs_coords.q(), qrs_coords.r());
    return {};
  }

  auto& region = data_.GetSurface().GetCell(qrs_coords).GetRegion();
  return RegionObject::make_region_info(region);
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


