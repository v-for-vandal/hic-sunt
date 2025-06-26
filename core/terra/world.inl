
#include <core/utils/serialize.hpp>

namespace hs::terra {


template<typename BaseTypes>
bool World<BaseTypes>::operator==(const World& other) const {
  if(this == &other) {
    return true;
  }

  if (planes_ != other.planes_) {
    return false;
  }

  return true;

}

template<typename BaseTypes>
auto World<BaseTypes>::GetPlane(const StringId& plane_id) const -> PlanePtr {
  if(auto fit = planes_.find(plane_id); fit != planes_.end()) {
    return fit->second;
  }

  spdlog::info("Can't find Plane with id {}", plane_id);

  return {};
}

template<typename BaseTypes>
auto World<BaseTypes>::AddPlane(const StringId& plane_id, QRSBox box, int region_radius) -> PlanePtr {
  if(auto fit = planes_.find(plane_id); fit != planes_.end()) {
      spdlog::error("Plane with id {} already exists", plane_id);
      return fit->second;
  }

  auto plane_ptr = std::make_shared<Plane>(box, region_radius);
  planes_[plane_id] = plane_ptr;

  return plane_ptr;
}

template<typename BaseTypes>
void SerializeTo(const World<BaseTypes>& source, proto::terra::World& target)
{
  target.set_id("world");
  for(auto& [k, v] : source.planes_) {
    if (v == nullptr) {
      continue;
    }
    auto dst = target.mutable_planes()->Add();
    SerializeTo(*v, *dst);
  }

}

template<typename BaseTypes>
World<BaseTypes> ParseFrom(const proto::terra::World& source, serialize::To<World<BaseTypes>>)
{
  World<BaseTypes> result;
  for(const auto& plane: source.planes()) {
    PlanePtr<BaseTypes> plane_obj = std::make_shared<Plane<BaseTypes>>(
      ParseFrom(plane, serialize::To<Plane<BaseTypes>>{})
      );
    const auto plane_id = plane_obj->GetPlaneId();
    result.planes_[plane_id] = plane_obj;
  }
  result.InitNonpersistent();
  return result;
}

template<typename BaseTypes>
void World<BaseTypes>::InitNonpersistent() {}

}

