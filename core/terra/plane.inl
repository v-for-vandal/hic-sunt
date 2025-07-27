#include <core/terra/plane.hpp>

#include <core/utils/serialize.hpp>

namespace hs::terra {

template<typename BaseTypes>
Plane<BaseTypes>::Plane(
    ControlObjectPtr control_object,
    StringId plane_id,
    QRSBox box,
    int region_radius,
    int external_region_radius
  ):
  control_object_(std::move(control_object)),
  plane_id_(plane_id),
  surface_(box.start().q(), box.end().q(), box.start().r(), box.end().r(),
    box.start().s(), box.end().s())
  {
      if (region_radius <= 0) {
          region_radius = 5;
      }

      if (external_region_radius < region_radius) {
          external_region_radius = region_radius + region_radius / 2 + 1;
      }
      external_region_radius_ = external_region_radius;
      // Init regions
      GetSurface().foreach([region_radius, control_object(this->control_object_)](QRSCoords, Cell& cell) {
          auto region_id = BaseTypes::StringIdFromStdString(fmt::format("rgn_{}", control_object->GetNextId()));
          cell.SetRegion(Region(region_id, region_radius));
          });

      InitNonpersistent();
  }


template<typename BaseTypes>
bool Plane<BaseTypes>::operator==(const Plane& other) const {
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

template<typename BaseTypes>
void Plane<BaseTypes>::InitNonpersistent() {
  region_index_.clear();

  for(auto q = surface_.q_start(); q != surface_.q_end(); q++) {
    for(auto r = surface_.r_start(); r != surface_.r_end(); r++) {
      if(surface_.Contains(q,r)) {
        auto region_ptr = surface_.GetCell(q,r).GetRegionPtr();
        auto region_id = region_ptr->GetId();
        region_index_[region_id] = region_ptr;
      }
    }
  }

  for(const auto& cell : off_surface_) {
      auto region_ptr = cell.GetRegionPtr();
      auto region_id = region_ptr->GetId();
      region_index_[region_id] = region_ptr;
  }

}

template<typename BaseTypes>
auto Plane<BaseTypes>::GetRegionById(const StringId& region_id) const -> RegionPtr {
  if(auto fit = region_index_.find(region_id); fit != region_index_.end()) {
    return fit->second;
  }

  spdlog::info("Can't find region {}", region_id);

  return {};
}

template<typename BaseTypes>
auto Plane<BaseTypes>::GetRegion(QRSCoords coords) const -> RegionPtr {
    if (!surface_.Contains(coords)) {
        return nullptr;
    }

    return surface_.GetCell(coords).GetRegionPtr();
}

template<typename BaseTypes>
void Plane<BaseTypes>::SetRegion(QRSCoords coords, Region region) {
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

template<typename BaseTypes>
float Plane<BaseTypes>::GetDistanceBetweenCells(QRSCoords region1_coords, QRSCoords cell1,
      QRSCoords region2_coords, QRSCoords cell2)
{
    auto region1 = GetRegion(region1_coords);
    auto region2 = GetRegion(region2_coords);

    if (!region1) {
        spdlog::info("Region at {} does not exist", region1_coords);
        return -1;
    }
    if (!region2) {
        spdlog::info("Region at {} does not exist", region2_coords);
        return -1;
    }

    if(!region1->GetSurface().Contains(cell1)) {
        spdlog::info("Region at {} does not contain cell {}", region1_coords, cell1);
        return -1;
    }
    if(!region2->GetSurface().Contains(cell2)) {
        spdlog::info("Region at {} does not contain cell {}", region2_coords, cell2);
        return -2;
    }

    // TODO: Consider wrapping

    // put everything into one coordinate system
    auto total_cell1_coords = region1_coords * external_region_radius_ + cell1.AsDelta();
    auto total_cell2_coords = region2_coords * external_region_radius_ + cell2.AsDelta();

    auto delta = total_cell1_coords - total_cell2_coords;

    const auto result = sqrt(
        delta.q().ToUnderlying() * delta.q().ToUnderlying() +
        delta.r().ToUnderlying() * delta.r().ToUnderlying() +
        delta.s().ToUnderlying() * delta.s().ToUnderlying()
        );

    // TODO: RM
    /*
    spdlog::info("Distance between {}/{} and {}/{} is {}",
        region1_coords, cell1,
        region2_coords, cell2,
        result
        );
        */

    return result;
}

template<typename BaseTypes>
void SerializeTo(const Plane<BaseTypes>& source, proto::terra::Plane& target)
{
  target.set_id(BaseTypes::ToProtoString(source.plane_id_));
  target.set_external_region_radius(source.external_region_radius_);
  SerializeTo(source.surface_, *target.mutable_surface());
  hs::SerializeTo(source.off_surface_, *target.mutable_off_surface());
}

template<typename BaseTypes>
Plane<BaseTypes> ParseFrom(const proto::terra::Plane& source, serialize::To<Plane<BaseTypes>>)
{
  using StringId = typename BaseTypes::StringId;
  Plane<BaseTypes> result;
  result.plane_id_ = ParseFrom(source.id(), serialize::To<StringId>{});
  result.external_region_radius_ = source.external_region_radius();
  if(source.has_surface()) {
    result.surface_ = ParseFrom(source.surface(), serialize::To<typename Plane<BaseTypes>::Surface>{});
  }
  result.InitNonpersistent();
  return result;
}


}
