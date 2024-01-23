#include "world.hpp"

#include <core/utils/serialize.hpp>

#include <gtest/gtest.h>

namespace hs::terra {

using namespace ::hs::geometry::literals;

TEST(World, Serialize) {
  World ref_world(4_dq, 4_dr);

  ::flatbuffers::FlatBufferBuilder fbb{};
  auto offset = SerializeTo(ref_world, fbb);
  fbb.Finish(offset);

  auto data_ptr = fbb.GetBufferPointer();
  auto data_size = fbb.GetSize();

  const fbs::World* fbs_world = fbs::GetWorld(data_ptr);

  ASSERT_NE(fbs_world, nullptr);

  auto parse_world = ParseFrom(*fbs_world, serialize::To<World>{});

  // TODO: Compare ref and parse

}


}
