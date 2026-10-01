#include <gtest/gtest.h>
#include <terra/world.pb.h>

#include <core/utils/serialize.hpp>
#include <stdexcept>

#include "world.hpp"

namespace hs::terra {

using namespace ::hs::geometry::literals;
using StdWorld = World<>;
using ScopeType = types::ScopeType;

TEST(StdWorldSerialize, RejectsScopeWithMissingParent) {
  proto::terra::World proto_world;
  proto_world.set_scope_id("child.scope");

  auto* child_scope = proto_world.add_scopes();
  child_scope->set_id("child.scope");
  child_scope->set_scope_type(ScopeType::SCOPE_TYPE_PLANE);
  child_scope->set_parent_scope_id("missing.parent");

  EXPECT_THROW(ParseFrom(proto_world, serialize::To<StdWorld>{}), std::runtime_error);
}

TEST(StdWorldSerialize, RejectsScopeWithMissingTag) {
  proto::terra::World proto_world;
  proto_world.set_scope_id("source.scope");

  auto* source_scope = proto_world.add_scopes();
  source_scope->set_id("source.scope");
  source_scope->set_scope_type(ScopeType::SCOPE_TYPE_IMPROVEMENT);
  source_scope->add_tag_scope_ids("missing.tag");

  EXPECT_THROW(ParseFrom(proto_world, serialize::To<StdWorld>{}), std::runtime_error);
}

TEST(StdWorldSerialize, RejectsScopeCycles) {
  proto::terra::World proto_world;
  proto_world.set_scope_id("scope.a");

  auto* scope_a = proto_world.add_scopes();
  scope_a->set_id("scope.a");
  scope_a->set_scope_type(ScopeType::SCOPE_TYPE_IMPROVEMENT);
  scope_a->add_tag_scope_ids("scope.b");

  auto* scope_b = proto_world.add_scopes();
  scope_b->set_id("scope.b");
  scope_b->set_scope_type(ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS);
  scope_b->add_tag_scope_ids("scope.a");

  EXPECT_THROW(ParseFrom(proto_world, serialize::To<StdWorld>{}), std::runtime_error);
}

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
