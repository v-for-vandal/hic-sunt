#include <core/terra/world.hpp>
#include <core/terra/plane.hpp>

#include <core/utils/serialize.hpp>

namespace hs::terra {


bool World::operator==(const World& other) const {
  if(this == &other) {
    return true;
  }

  if (planes_ != other.planes_) {
    return false;
  }

  return true;

}

PlanePtr World::GetPlane(PlaneIdCRef plane_id) const {
  if(auto fit = planes_.find(plane_id); fit != planes_.end()) {
    return fit->second;
  }

  spdlog::info("Can't find region {}", plane_id);

  return {};
}

void SerializeTo(const World& source, proto::terra::World& target)
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

World ParseFrom(const proto::terra::World& source, serialize::To<World>)
{
  World result;
  for(const auto& plane: source.planes()) {
    PlanePtr plane_obj = std::make_shared<Plane>(
      ParseFrom(plane, serialize::To<Plane>{})
      );
    const auto plane_id = plane_obj->GetPlaneId();
    result.planes_[plane_id] = plane_obj;
  }
  result.InitNonpersistent();
  return result;
}

void World::InitNonpersistent() {}

}
