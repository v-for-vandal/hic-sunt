#include <core/terra/world.hpp>

#include <fbs/world_generated.h>

namespace hs::terra {

::flatbuffers::Offset<fbs::World> SerializeTo(const World& source,  ::flatbuffers::FlatBufferBuilder& fbb)
{
  hs::fbs::WorldBuilder builder(fbb);
  builder.add_id(fbb.CreateString("test_id"));
  builder.add_surface(SerializeTo(source.surface_, fbb, serialize::To<fbs::SurfaceBuilder>{}));
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
}
