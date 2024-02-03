#include "world.hpp"

#include <core/utils/serialize.hpp>

#include <gtest/gtest.h>

namespace hs::terra {

using namespace ::hs::geometry::literals;

TEST(World, Serialize) {
  World ref_world(4_dq, 4_dr);

  std::string storage;

  proto::terra::World proto_world;
  SerializeTo(ref_world, proto_world);
  proto_world.SerializeToString(&storage);


  proto::terra::World proto_read_world;
  ASSERT_TRUE(proto_read_world.ParseFromString(storage));

  auto parse_world = ParseFrom(proto_read_world, serialize::To<World>{});

  // TODO: Compare ref and parse

}


}
