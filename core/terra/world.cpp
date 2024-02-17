#include <core/terra/world.hpp>

#include <core/utils/serialize.hpp>

namespace hs::terra {

World::World(
    QRSCoordinateSystem::QAxis q_start,
    QRSCoordinateSystem::QAxis q_end,
    QRSCoordinateSystem::RAxis r_start,
    QRSCoordinateSystem::RAxis r_end,
    QRSCoordinateSystem::SAxis s_start,
    QRSCoordinateSystem::SAxis s_end
  ):
  surface_(q_start, q_end, r_start, r_end, s_start, s_end)
  {
    InitRegionIndex();
  }

/*
World::World(QRSSize size):
  surface_(size)
  {
    InitRegionIndex();
  }
  */

void World::InitRegionIndex() {
  for(auto q = surface_.q_start(); q != surface.q_end(); q++) {
    for(auto r = surface_.r_start(); r != surface.r_end(); r++) {
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

std::shared_ptr<region::Region> World::GetRegionById(const std::string& region_id) {
  if(auto fit = region_index_.find(region_id); fit != region_index_.end()) {
    return fit->second;
  }

  return {};
}

/*
::flatbuffers::Offset<fbs::World> SerializeTo(const World& source,  ::flatbuffers::FlatBufferBuilder& fbb)
{
  auto id_offset = fbb.CreateString("test_id");
  auto surface_offset = SerializeTo(source.surface_, fbb, serialize::To<fbs::SurfaceBuilder>{});
  hs::fbs::WorldBuilder builder(fbb);
  builder.add_id(id_offset);
  builder.add_surface(surface_offset);
  return builder.Finish();
}

World ParseFrom(const fbs::World& world, serialize::To<World>)
{
  World result;
  if(world.surface()) {
    result.surface_ = ParseFrom(*world.surface(), serialize::To<World::Surface>{});
  }

  return result;
}
*/

void SerializeTo(const World& source, proto::terra::World& target)
{
  target.set_id("test_id");
  SerializeTo(source.surface_, *target.mutable_surface());
  SerializeTo(source.off_surface_, *target.mutable_off_surface());
}

World ParseFrom(const proto::terra::World& source, serialize::To<World>)
{
  World result;
  if(source.has_surface()) {
    result.surface_ = ParseFrom(source.surface(), serialize::To<World::Surface>{});
  }
  return result;
}

}
