
#include <core/utils/serialize.hpp>

namespace hs::terra {

template <typename BaseTypes>
bool World<BaseTypes>::operator==(const World &other) const {
  if (this == &other) {
    return true;
  }

  if (planes_ != other.planes_) {
    return false;
  }

  return true;
}

template <typename BaseTypes>
auto World<BaseTypes>::GetPlane(const StringId &plane_id) const -> PlanePtr {
  if (auto fit = planes_.find(plane_id); fit != planes_.end()) {
    return fit->second;
  }

  spdlog::info("Can't find Plane with id {}", plane_id);

  return {};
}

template <typename BaseTypes>
auto World<BaseTypes>::AddPlane(const StringId &plane_id, QRSBox box,
                                int region_radius,
                                int region_external_radius) -> PlanePtr {
  if (auto fit = planes_.find(plane_id); fit != planes_.end()) {
    spdlog::error("Plane with id {} already exists", plane_id);
    return fit->second;
  }

  spdlog::info("Making new plane with dimensions: {} and radius: {}", box,
               region_radius);

  StringId effective_plane_id = plane_id;
  if (BaseTypes::IsNullToken(plane_id)) {
    effective_plane_id = BaseTypes::StringIdFromStdString(
        fmt::format("plane_{}", control_object_->GetNextId()));
  }

  auto plane_ptr =
      std::make_shared<Plane>(control_object_, effective_plane_id, box,
                              region_radius, region_external_radius);
  planes_[effective_plane_id] = plane_ptr;

  return plane_ptr;
}

template <typename BaseTypes>
auto World<BaseTypes>::GetRegionById(const StringId &region_id) const noexcept
    -> RegionPtr {
  RegionPtr result;
  for (auto &[_, plane_ptr] : planes_) {
    result = plane_ptr->GetRegionById(region_id);
    if (result != nullptr) {
      break;
    }
  }

  return result;
}

template <typename BaseTypes>
bool World<BaseTypes>::HasRegion(const StringId &region_id) const noexcept {
  for (auto &[_, plane_ptr] : planes_) {
    if (plane_ptr->HasRegion(region_id)) {
      return true;
    }
  }

  return false;
}

template <typename BaseTypes>
void SerializeTo(const World<BaseTypes> &source, proto::terra::World &target) {
  target.set_id("world");
  for (auto &[k, v] : source.planes_) {
    if (v == nullptr) {
      continue;
    }
    auto dst = target.mutable_planes()->Add();
    SerializeTo(*v, *dst);
  }

  SerializeTo(*source.control_object_, *target.mutable_control_object());
}

template <typename BaseTypes>
World<BaseTypes> ParseFrom(const proto::terra::World &source,
                           serialize::To<World<BaseTypes>>) {
  World<BaseTypes> result;
  for (const auto &plane : source.planes()) {
    PlanePtr<BaseTypes> plane_obj = std::make_shared<Plane<BaseTypes>>(
        ParseFrom(plane, serialize::To<Plane<BaseTypes>>{}));
    const auto plane_id = plane_obj->GetPlaneId();
    result.planes_[plane_id] = plane_obj;
  }
  *result.control_object_ =
      ParseFrom(source.control_object(), serialize::To<types::ControlObject>{});

  result.InitNonpersistent();
  return result;
}

template <typename BaseTypes> void World<BaseTypes>::InitNonpersistent() {
  // set control object
  for (auto &[_, plane_ptr] : planes_) {
    plane_ptr->SetControlObject(control_object_);
  }

  // TODO: build region index
}

} // namespace hs::terra
