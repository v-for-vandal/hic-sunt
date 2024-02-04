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

  auto& cell = surface_.GetCell(coords);
  if(!cell.GetTerrain().empty()) {
    terrain_count_[std::string{cell.GetTerrain()}]--;
  }
  cell.SetTerrain(terrain);
  terrain_count_[std::string{cell.GetTerrain()}]++;

  return true;
}

bool Region::SetFeature(QRSCoords coords, std::string_view feature)
{
  if(!surface_.Contains(coords)) {
    return false;
  }

  auto& cell = surface_.GetCell(coords);
  cell.SetFeature(feature);
  //cell.
}

bool Region::SetImprovement(QRSCoords /*coords*/, std::string_view /*improvement*/)
{
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

  return result;
}
}
