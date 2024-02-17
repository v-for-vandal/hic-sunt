#include "region.hpp"

namespace hs::region {

Region::Region(QRSSize size):
  surface_(size)
  {
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

void SerializeTo(const Region& source, proto::region::Region& target)
{
  target.Clear();
  SerializeTo(source.surface_, *target.mutable_surface());
}

Region ParseFrom(const proto::region::Region& region, serialize::To<Region>)
{
  Region result;
  if(region.has_surface()) {
    result.surface_ = ParseFrom(region.surface(), serialize::To<Region::Surface>{});
  }

  // Restore data
  auto surface = result.surface_.view();
  for(auto q = surface.q_start(); q != surface.q_end(); ++q) {
    for(auto r = surface.r_start(); r != surface.r_end(); ++r) {
      if(surface.Contains(q, r)) {
        auto& cell = surface.GetCell(q,r);
        result.terrain_count_.Add(std::string{cell.GetTerrain()});
        result.feature_count_[std::string{cell.GetFeature()}]++;
      }
    }
  }

  return result;
}
}
