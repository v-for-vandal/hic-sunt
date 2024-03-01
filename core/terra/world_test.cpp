#include "world.hpp"

#include <core/utils/serialize.hpp>

#include <gtest/gtest.h>

namespace hs::terra {

using namespace ::hs::geometry::literals;

TEST(World, Serialize) {
  World ref_world(-1_q, 4_q,  -2_r, 6_r, -1_s, 4_s);

  std::string storage;

  proto::terra::World proto_world;
  SerializeTo(ref_world, proto_world);
  proto_world.SerializeToString(&storage);


  proto::terra::World proto_read_world;
  ASSERT_TRUE(proto_read_world.ParseFromString(storage));

  auto parse_world = ParseFrom(proto_read_world, serialize::To<World>{});

  EXPECT_EQ(ref_world, parse_world);
  EXPECT_EQ(ref_world.GetSurfaceObject().data_size(), parse_world.GetSurfaceObject().data_size());
}


}
