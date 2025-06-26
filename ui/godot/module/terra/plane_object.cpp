#include "plane_object.hpp"

#include <fstream>

#include <ui/godot/module/utils/to_string.hpp>
#include <ui/godot/module/utils/cast_qrs.hpp>

namespace hs::godot {

void PlaneObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_dimensions"), &PlaneObject::get_dimensions);
  ClassDB::bind_method(D_METHOD("get_region", "coords"), &PlaneObject::get_region);
  ClassDB::bind_method(D_METHOD("get_region_by_id", "region_id"), &PlaneObject::get_region_by_id);
  ClassDB::bind_method(D_METHOD("contains", "coords"), &PlaneObject::contains);
  ClassDB::bind_method(D_METHOD("get_region_info", "coords"), &PlaneObject::get_region_info);
  ClassDB::bind_method(D_METHOD("foreach_surface", "callback"), &PlaneObject::foreach_surface);
}

Rect2i PlaneObject::get_dimensions() const {
  return Rect2i{
    Vector2i{
      data_->GetSurface().q_start().ToUnderlying(),
      data_->GetSurface().r_start().ToUnderlying()
    },
    Vector2i{
      data_->GetSurface().q_size().ToUnderlying(),
      data_->GetSurface().r_size().ToUnderlying()
    }
  };
}

Ref<RegionObject> PlaneObject::get_region(Vector2i coords) const {
  auto qrs_coords = cast_qrs(coords);

  ERR_FAIL_COND_V_MSG(!data_->GetSurface().Contains(qrs_coords),
    Ref<RegionObject>{}, utils::to_string(fmt::format("no region at coords {}", qrs_coords)));

  /* TODO: check above should take care of that
  if (!data_->GetSurface().Contains(qrs_coords)) {
    SPDLOG_INFO("no region at coords {},{}", qrs_coords.q(), qrs_coords.r());
    return {};
  }
  */

  Ref<RegionObject> result(memnew(RegionObject(
      data_->GetSurface().GetCell(qrs_coords).GetRegionPtr()
        )));

  return result;
}

Ref<RegionObject> PlaneObject::get_region_by_id(String region_id) const {
  ERR_FAIL_COND_V_MSG(!data_->HasRegion(region_id),
    Ref<RegionObject>{}, utils::to_string(fmt::format("no region with id {}", region_id)));

  Ref<RegionObject> result(memnew(RegionObject(
      data_->GetRegionById(region_id)
        )));

  return result;
}

void PlaneObject::foreach_surface(const Callable& callback) {
  data_->GetSurface().foreach(
    [&callback, data(this->data_)](QRSCoords coords, Cell& cell) {
      Ref<RegionObject> region(memnew(RegionObject(
          data->GetSurface().GetCell(coords).GetRegionPtr()
            )));
      callback.call(coords.q().ToUnderlying(), coords.r().ToUnderlying(), region);
    });
}

bool PlaneObject::contains(Vector2i coords) const {
  auto qrs_coords = cast_qrs(coords);

  return data_->GetSurface().Contains(qrs_coords);
}

Dictionary PlaneObject::get_region_info(Vector2i coords) const {
  auto qrs_coords = cast_qrs(coords);

  if (!data_->GetSurface().Contains(qrs_coords)) {
    SPDLOG_INFO("no region at coords {},{}", qrs_coords.q(), qrs_coords.r());
    return {};
  }

  auto& region = data_->GetSurface().GetCell(qrs_coords).GetRegion();
  return RegionObject::make_region_info(region);
}

Dictionary PlaneObject::create_error(const char* error) {
    Dictionary result;
    result["success"] = false;
    result["error"] = error;
    return result;

}

Dictionary PlaneObject::create_success() {
    Dictionary result;
    result["success"] = true;
    return result;
}


#if 0

void PlaneObject::set_world(std::shared_ptr<terra::Plane> world_ptr) {
  /*
  world_ptr_ = std::move(world_ptr);
  start_point_ = Vector3FromCoords(world_ptr_->begin_point());
  end_point_ = Vector3FromCoords(world_ptr_->end_point());
  mc_start_point_ = Vector3FromCoords(world_ptr_->GetMainCellsView().begin_point());
  mc_end_point_ = Vector3FromCoords(world_ptr_->GetMainCellsView().end_point());
  */
}

Ref<CellObject> PlaneObject::GetCellObject(Vector3 godot_coords) const {
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


}
