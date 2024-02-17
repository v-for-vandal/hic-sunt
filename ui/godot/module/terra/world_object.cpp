#include "world_object.hpp"

#include <ui/godot/module/utils/to_string.hpp>

void WorldObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_dimensions"), &WorldObject::get_dimensions);
  ClassDB::bind_method(D_METHOD("get_region", "coords"), &WorldObject::get_region);
  ClassDB::bind_method(D_METHOD("contains", "coords"), &WorldObject::contains);
  ClassDB::bind_method(D_METHOD("get_region_info", "coords"), &WorldObject::get_region_info);
}

Vector2i WorldObject::get_dimensions() const {
  return Vector2i{
    data_.GetSurface().q_size().ToUnderlying(),
    data_.GetSurface().r_size().ToUnderlying()
  };
}

Ref<RegionObject> WorldObject::get_region(Vector2i coords) const {
  auto qrs_coords = cast_qrs(coords);

  ERR_FAIL_COND_V_MSG(!data_.GetSurface().Contains(qrs_coords),
    Ref<RegionObject>{}, utils::to_string(fmt::format("no region at coords {}", qrs_coords)));

  /* TODO: check above should take care of that
  if (!data_.GetSurface().Contains(qrs_coords)) {
    SPDLOG_INFO("no region at coords {},{}", qrs_coords.q(), qrs_coords.r());
    return {};
  }
  */

  Ref<RegionObject> result(memnew(RegionObject(
      data_.GetSurface().GetCell(qrs_coords).GetRegionPtr()
        )));

  return result;
}

Ref<RegionObject> WorldObject::get_region_by_id(String region_id) const {
  std::string std_region_id = region_id.utf8().get_data();
  ERR_FAIL_COND_V_MSG(!data_.HasRegion(std_region_id),
    Ref<RegionObject>{}, utils::to_string(fmt::format("no region with id {}", std_region_id)));

  Ref<RegionObject> result(memnew(RegionObject(
      data_.GetRegionById(std_region_id)
        )));
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


