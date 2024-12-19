#include <core/terra/plane.hpp>

#include <core/utils/serialize.hpp>

namespace hs::terra {

Plane::Plane(
    QRSBox box
  ):
  plane_id_(fmt::format("plane_{}", next_id_++)),
  surface_(box.start().q(), box.end().q(), box.start().r(), box.end().r(),
    box.start().s(), box.end().s())
  {
    InitNonpersistent();
  }


bool Plane::operator==(const Plane& other) const {
  if(this == &other) {
    return true;
  }

  if(surface_ != other.surface_) {
    SPDLOG_TRACE("surface not equal");
    return false;
  }

  if(off_surface_ != other.off_surface_) {
    SPDLOG_TRACE("off-surface not equal");
    return false;
  }

  if(region_index_.size() != other.region_index_.size()) {
    SPDLOG_TRACE("region index not equal (by size)");
    return false;
  }

  for(auto& [k,v] : region_index_) {
    auto other_fit = other.region_index_.find(k);
    if(other_fit == other.region_index_.end()) {
      SPDLOG_TRACE("region index not equal, no key {} in other", k);
      return false;
    }

    if(*v != *(other_fit->second)) {
      SPDLOG_TRACE("region index not equal, values differ, key \"{}\"", k);
      return false;
    }
  }

  return true;

}

void Plane::InitNonpersistent() {
  region_index_.clear();

  for(auto q = surface_.q_start(); q != surface_.q_end(); q++) {
    for(auto r = surface_.r_start(); r != surface_.r_end(); r++) {
      if(surface_.Contains(q,r)) {
        auto region_ptr = surface_.GetCell(q,r).GetRegionPtr();
        auto region_id = region_ptr->GetId();
        region_index_[std::string{region_id}] = region_ptr;
      }
    }
  }

  for(const auto& cell : off_surface_) {
      auto region_ptr = cell.GetRegionPtr();
      auto region_id = region_ptr->GetId();
      region_index_[std::string{region_id}] = region_ptr;
  }

}

std::shared_ptr<region::Region> Plane::GetRegionById(const std::string& region_id) const {
  if(auto fit = region_index_.find(region_id); fit != region_index_.end()) {
    return fit->second;
  }

  spdlog::info("Can't find region {}", region_id);

  return {};
}

void Plane::SetRegion(QRSCoords coords, region::Region region) {
  if(!surface_.Contains(coords)) {
    return;
  }

  auto& cell = surface_.GetCell(coords);

  region_index_.erase(
    std::string{cell.GetRegion().GetId()});

  cell.SetRegion(std::move(region));

  auto new_region_ptr = cell.GetRegionPtr();

  region_index_.try_emplace(std::string{new_region_ptr->GetId()}, new_region_ptr);

}

void SerializeTo(const Plane& source, proto::terra::Plane& target)
{
  target.set_id("test_id");
  SerializeTo(source.surface_, *target.mutable_surface());
  hs::SerializeTo(source.off_surface_, *target.mutable_off_surface());
}

Plane ParseFrom(const proto::terra::Plane& source, serialize::To<Plane>)
{
  Plane result;
  if(source.has_surface()) {
    result.surface_ = ParseFrom(source.surface(), serialize::To<Plane::Surface>{});
  }
  result.InitNonpersistent();
  return result;
}

}

