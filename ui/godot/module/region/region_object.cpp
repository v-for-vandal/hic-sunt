#include "region_object.hpp"

void RegionObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_dimensions"), &RegionObject::get_dimensions);
  ClassDB::bind_method(D_METHOD("get_cell_info", "coords"), &RegionObject::get_cell_info);
  ClassDB::bind_method(D_METHOD("set_terrain", "coords", "terrain"), &RegionObject::set_terrain);
  ClassDB::bind_method(D_METHOD("set_feature", "coords", "feature"), &RegionObject::set_feature);
}

Vector2i RegionObject::get_dimensions() const {
  if(!region_) {
    return Vector2i{0,0};
  }

  return Vector2i{
    region_->GetSurface().q_size().ToUnderlying(),
    region_->GetSurface().r_size().ToUnderlying()
  };
}

Dictionary RegionObject::get_cell_info(Vector2i coords) const {
  if(!region_) {
    return {};
  }
  auto qrs_coords = to_qrs(coords);

  if (!region_->GetSurface().Contains(qrs_coords)) {
    return {};
  }

  auto& cell = region_->GetSurface().GetCell(qrs_coords);

  // Fill result
  Dictionary result;
  result["terrain"] = cell.GetTerrain().data();
  result["feature"] = cell.GetFeature().data();

  return result;
}

bool RegionObject::set_terrain(Vector2i coords, String terrain) const
{
  if(!region_) {
    return false;
  }
  auto qrs_coords = to_qrs(coords);

  if (!region_->GetSurface().Contains(qrs_coords)) {
    return false;
  }

  return region_->SetTerrain(qrs_coords, terrain.utf8().get_data());
}

bool RegionObject::set_feature(Vector2i coords, String feature) const
{
  if(!region_) {
    return false;
  }
  auto qrs_coords = to_qrs(coords);

  if (!region_->GetSurface().Contains(qrs_coords)) {
    return false;
  }

  return region_->SetFeature(qrs_coords, feature.utf8().get_data());
}
