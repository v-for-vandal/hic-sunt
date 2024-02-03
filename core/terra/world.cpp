#include <core/terra/world.hpp>

#include <fbs/world_generated.h>

namespace hs::terra {

World::World(QRSCoordinateSystem::QDelta q_size, QRSCoordinateSystem::RDelta r_size):
  surface_(q_size, r_size)
  {
  }

World::World(QRSSize size):
  surface_(size)
  {
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
