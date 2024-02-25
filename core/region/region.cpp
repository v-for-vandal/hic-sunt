#include "region.hpp"

namespace hs::region {

Region::Region():
  Region(1)
{
}

Region::Region(int radius):
  id_(fmt::format("region_{}", next_id_++)),
  surface_(
    QRSCoordinateSystem::QAxis(-radius),
    QRSCoordinateSystem::QAxis(radius),
    QRSCoordinateSystem::RAxis(-radius),
    QRSCoordinateSystem::RAxis(radius),
    QRSCoordinateSystem::SAxis(-radius),
    QRSCoordinateSystem::SAxis(radius)
    )
  {
    InitNonpersistent();
  }

void Region::InitNonpersistent() {
  terrain_count_.clear();
  feature_count_.clear();
  auto surface = surface_.view();
  for(auto q = surface.q_start(); q != surface.q_end(); ++q) {
    for(auto r = surface.r_start(); r != surface.r_end(); ++r) {
      if(surface.Contains(q, r)) {
        auto& cell = surface.GetCell(q,r);
        terrain_count_.Add(std::string{cell.GetTerrain()});
        feature_count_[std::string{cell.GetFeature()}]++;
      }
    }
  }

}

bool Region::operator==(const Region& other) const {
  if(this == &other) {
    return true;
  }

  if(id_ != other.id_) {
    SPDLOG_TRACE("id not equal, {} vs {}", id_, other.id_);
    return false;
  }

  if(city_id_ != other.city_id_) {
    SPDLOG_TRACE("id not equal, {} vs {}", city_id_, other.city_id_);
    return false;
  }

  if(surface_ != other.surface_) {
    SPDLOG_TRACE("surface not equal");
    return false;
  }

  if(terrain_count_ != other.terrain_count_) {
    SPDLOG_TRACE("terrain_count_ comb not equal");
    return false;
  }

  if(feature_count_ != other.feature_count_) {
    SPDLOG_TRACE("feature_count_ not equal");
    return false;
  }

  return true;
}


bool Region::SetTerrain(QRSCoords coords, std::string_view terrain)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  std::string terrain_str{terrain};

  auto& cell = surface_.GetCell(coords);
  if(!cell.GetTerrain().empty()) {
    terrain_count_.Remove(std::string{cell.GetTerrain()});
  }
  cell.SetTerrain(terrain);
  terrain_count_.Add(terrain_str);

  return true;
}

bool Region::SetFeature(QRSCoords coords, std::string_view feature)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  auto& cell = surface_.GetCell(coords);
  cell.SetFeature(feature);
  return true;
  //cell.
}


bool Region::SetImprovement(QRSCoords coords, std::string_view improvement)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  auto& cell = surface_.GetCell(coords);
  cell.SetImprovement(improvement);
  return true;
}


bool Region::SetCityId(std::string_view city_id) {
  if(IsCity() && !city_id.empty()) {
    spdlog::error("Can't set city_id in region where city is already present");
    return false;
  }

  city_id_ = city_id;
  return true;
}

void SerializeTo(const Region& source, proto::region::Region& target)
{
  target.Clear();
  SerializeTo(source.surface_, *target.mutable_surface());
  target.set_id(source.id_);
  target.set_city_id(source.city_id_);
}

Region ParseFrom(const proto::region::Region& region, serialize::To<Region>)
{
  Region result;
  if(region.has_surface()) {
    result.surface_ = ParseFrom(region.surface(), serialize::To<Region::Surface>{});
  }
  result.id_ = region.id();
  result.city_id_ = region.city_id();

  // Restore non-persistent data
  result.InitNonpersistent();

  return result;
}
}
