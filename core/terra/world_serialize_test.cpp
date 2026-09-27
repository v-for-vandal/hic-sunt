#include <gtest/gtest.h>
#include <terra/world.pb.h>

#include <core/utils/serialize.hpp>

#include "world.hpp"

namespace hs::terra {

using namespace ::hs::geometry::literals;
using StdWorld = World<>;
using ScopeType = types::ScopeType;

TEST(StdWorldSerialize, RoundTripPreservesPlanesCivilizationsAndControlObject) {
  StdWorld source;
  auto plane = source.AddPlane(
      "plane.alpha", StdWorld::QRSBox(StdWorld::QRSCoords(0_q, 0_r), StdWorld::QRSCoords(0_q, 0_r)),
      1, 2);
  ASSERT_NE(plane, nullptr);
  auto civilization = source.GetOrCreateCivilization("civ.alpha");
  ASSERT_TRUE(civilization->CreateChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha").has_value());
  const auto next_id_before_save = source.GetNextId();

  proto::terra::World proto_world;
  SerializeTo(source, proto_world);

  auto parsed = ParseFrom(proto_world, serialize::To<StdWorld>{});

  EXPECT_NE(parsed.GetPlane("plane.alpha"), nullptr);
  EXPECT_TRUE(parsed.HasCivilization("civ.alpha"));
  EXPECT_TRUE(
      parsed.GetCivilization("civ.alpha")->HasChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha"));
  EXPECT_EQ(parsed.GetNextId(), next_id_before_save + 1);
}

}  // namespace hs::terra
