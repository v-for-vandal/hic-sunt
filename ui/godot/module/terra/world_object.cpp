#include "world_object.hpp"

#include <fstream>

#include <ui/godot/module/utils/to_string.hpp>

void WorldObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_dimensions"), &WorldObject::get_dimensions);
  ClassDB::bind_method(D_METHOD("get_region", "coords"), &WorldObject::get_region);
  ClassDB::bind_method(D_METHOD("get_region_by_id", "region_id"), &WorldObject::get_region_by_id);
  ClassDB::bind_method(D_METHOD("contains", "coords"), &WorldObject::contains);
  ClassDB::bind_method(D_METHOD("get_region_info", "coords"), &WorldObject::get_region_info);
  ClassDB::bind_method(D_METHOD("save", "filename"), &WorldObject::save);
  ClassDB::bind_method(D_METHOD("load", "filename"), &WorldObject::load);

  ClassDB::bind_static_method("WorldObject", D_METHOD("create_world", "size", "region_size"), &WorldObject::create_world);
}

Rect2i WorldObject::get_dimensions() const {
  return Rect2i{
    Vector2i{
      data_.GetSurface().q_start().ToUnderlying(),
      data_.GetSurface().r_start().ToUnderlying()
    },
    Vector2i{
      data_.GetSurface().q_size().ToUnderlying(),
      data_.GetSurface().r_size().ToUnderlying()
    }
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

Error WorldObject::save(String filename) {
  hs::proto::terra::World proto_world;
  SerializeTo(data_, proto_world);

  try {
    std::fstream output(filename.utf8().get_data(), std::ios::out | std::ios::trunc | std::ios::binary);
    if (!proto_world.SerializeToOstream(&output)) {
      spdlog::error("Can't write filename {} as proto object", filename.utf8().get_data());
      return ERR_FILE_CANT_WRITE;
    }
  } catch(const std::exception& e) {
    return ERR_FILE_CANT_OPEN;
  }
  return OK;
}

Error WorldObject::load(String filename) {
  hs::proto::terra::World proto_world;

  try {
    std::fstream input(filename.utf8().get_data(), std::ios::in | std::ios::binary);
    if (!proto_world.ParseFromIstream(&input)) {
      spdlog::error("Can't read filename {} as proto object", filename.utf8().get_data());
      return ERR_FILE_CANT_READ;
    }
  } catch(const std::exception& e) {
    return ERR_FILE_CANT_OPEN;
  }
  try {
    data_ = ParseFrom(proto_world, ::hs::serialize::To<hs::terra::World>{});
  } catch (const std::exception&e) {
    return ERR_FILE_CORRUPT;
  }


  return OK;

}

Dictionary WorldObject::create_error(const char* error) {
    Dictionary result;
    result["success"] = false;
    result["error"] = error;
    return result;

}

Dictionary WorldObject::create_success() {
    Dictionary result;
    result["success"] = true;
    return result;
}

Ref<WorldObject> WorldObject::create_world(Vector2i world_size, int region_radius) {

  if(region_radius <= 10) {
    region_radius = 10;
  }

  auto qrs_world_size = cast_qrs_size(world_size);

  auto world = hs::terra::World{
      hs::terra::World::QRSCoordinateSystem::QAxis{0},
      hs::terra::World::QRSCoordinateSystem::QAxis{0} + qrs_world_size.q(),
      hs::terra::World::QRSCoordinateSystem::RAxis{0},
      hs::terra::World::QRSCoordinateSystem::RAxis{0} + qrs_world_size.r(),
      hs::terra::World::QRSCoordinateSystem::SAxis{0},
      hs::terra::World::QRSCoordinateSystem::SAxis{0} + qrs_world_size.s()};

  auto surface = world.GetSurface();
  /* randomize terrain for now */
  for(auto q_pos = surface.q_start(); q_pos < surface.q_end(); q_pos++) {
    for( auto r_pos = surface.r_start(); r_pos < surface.r_end(); r_pos++) {
      auto coords = hs::terra::World::QRSCoords{q_pos, r_pos};
      if(surface.Contains(coords)) {
        // initialize region
        hs::region::Region region{region_radius};
        world.SetRegion(coords, std::move(region));
      }
    }
  }

  Ref<WorldObject> result(memnew(WorldObject(
        std::move(world))));

  ERR_FAIL_NULL_V_MSG(result.ptr(), result, "Failed to create new world");
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


