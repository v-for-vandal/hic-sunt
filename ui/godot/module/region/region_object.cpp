#include "region_object.hpp"

void RegionObject::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_dimensions"), &RegionObject::get_dimensions);
  ClassDB::bind_method(D_METHOD("get_region_id"), &RegionObject::get_region_id);
  ClassDB::bind_method(D_METHOD("get_city_id"), &RegionObject::get_city_id);
  ClassDB::bind_method(D_METHOD("set_city_id", "city_id"), &RegionObject::set_city_id);
  ClassDB::bind_method(D_METHOD("get_cell_info", "coords"), &RegionObject::get_cell_info);
  ClassDB::bind_method(D_METHOD("contains", "coords"), &RegionObject::contains);
  ClassDB::bind_method(D_METHOD("set_terrain", "coords", "terrain"), &RegionObject::set_terrain);
  ClassDB::bind_method(D_METHOD("set_feature", "coords", "feature"), &RegionObject::set_feature);
  ClassDB::bind_method(D_METHOD("get_available_improvements"), &RegionObject::get_available_improvements);
}

Rect2i RegionObject::get_dimensions() const {
  if(!region_) {
    return Rect2i{
      Vector2i{0,0},
      Vector2i{1,1}
    };
  }

  return Rect2i{
    Vector2i{
      region_->GetSurface().q_start().ToUnderlying(),
      region_->GetSurface().r_start().ToUnderlying()
    },
    Vector2i {
      region_->GetSurface().q_size().ToUnderlying(),
      region_->GetSurface().r_size().ToUnderlying()
    }
  };
}

Dictionary RegionObject::get_cell_info(Vector2i coords) const {
  if(!region_) {
    return {};
  }
  auto qrs_coords = cast_qrs(coords);

  if (!region_->GetSurface().Contains(qrs_coords)) {
    return {};
  }

  auto& cell = region_->GetSurface().GetCell(qrs_coords);

  // Fill result
  Dictionary result;
  result["terrain"] = cell.GetTerrain().data();
  result["feature"] = cell.GetFeature().data();
  result["improvement"] = cell.GetImprovement().data();

  return result;
}

bool RegionObject::set_terrain(Vector2i coords, String terrain) const
{
  if(!region_) {
    return false;
  }
  auto qrs_coords = cast_qrs(coords);

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
  auto qrs_coords = cast_qrs(coords);

  if (!region_->GetSurface().Contains(qrs_coords)) {
    return false;
  }

  return region_->SetFeature(qrs_coords, feature.utf8().get_data());
}

bool RegionObject::set_improvement(Vector2i coords, String improvement) const
{
  if(!region_) {
    return false;
  }
  auto qrs_coords = cast_qrs(coords);

  // Region object will perform checks that coords are valid and so
  // on.
  // However, it will not perform checks that this improvement follows
  // the ruleset
  return region_->SetImprovement(qrs_coords, improvement.utf8().get_data());
}

Dictionary RegionObject::make_region_info(const hs::region::Region& region) {
  Dictionary result;
  // Build top terrain
  {
    Array top_terrain_result;
    auto top_terrain = region.GetTopKTerrain(3);
    for(auto& [terrain, count] : top_terrain) {
      top_terrain_result.append(count);
      top_terrain_result.append(String(terrain.c_str()));
    }
    result["top_terrain"] = std::move(top_terrain_result);
  }
  //result["size"] = region.GetSurface().size();

  return result;
}

Array RegionObject::get_available_improvements() const {
  return {};
}

bool RegionObject::contains(Vector2i coords) const {
  auto qrs_coords = cast_qrs(coords);

  if(!region_) {
    return false;
  }

  return region_->GetSurface().Contains(qrs_coords);
}

String RegionObject::get_region_id() const {
  if(!region_) {
    return {};
  }

  return region_->GetId().data();

}

String RegionObject::get_city_id() const {
  ERR_FAIL_NULL_V_MSG(region_, String{}, "null-containing region object");

  return region_->GetCityId().data();
}

bool RegionObject::set_city_id(String city_id) const {
  ERR_FAIL_NULL_V_MSG(region_, false, "null-containing region object");

  return region_->SetCityId(city_id.utf8().get_data());
}

