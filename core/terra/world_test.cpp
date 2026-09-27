#include "world.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <core/types/scope_type.hpp>
#include <core/utils/serialize.hpp>

namespace hs::terra {

using namespace ::hs::geometry::literals;

using StdWorld = World<>;
using ScopeType = types::ScopeType;

TEST(World, Serialize) {}

TEST(StdWorld, VisitScopesVisitsOwnScopeThenNestedScopes) {
  StdWorld world;
  auto plane = world.AddPlane(
      "plane.alpha", StdWorld::QRSBox(StdWorld::QRSCoords(0_q, 0_r), StdWorld::QRSCoords(0_q, 0_r)),
      1, 2);
  auto civilization = world.GetOrCreateCivilization("civ.alpha");
  auto city_scope = civilization->CreateChildScope(ScopeType::SCOPE_TYPE_CITY, "city.alpha");

  ASSERT_NE(plane, nullptr);
  ASSERT_NE(civilization, nullptr);
  ASSERT_TRUE(city_scope.has_value());

  std::vector<std::string> scope_ids;
  world.VisitScopes([&scope_ids](const auto& scope_ptr) {
    ASSERT_NE(scope_ptr, nullptr);
    scope_ids.push_back(std::string{scope_ptr->GetId()});
  });

  size_t expected_region_count = 0;
  size_t expected_cell_count = 0;
  plane->GetSurface().Foreach([&](auto, auto& world_cell) {
    ++expected_region_count;
    world_cell.GetRegion().GetSurface().Foreach([&](auto, auto&) { ++expected_cell_count; });
  });

  ASSERT_EQ(scope_ids.size(), 1 + 1 + expected_region_count + expected_cell_count + 1 + 1);
  EXPECT_EQ(scope_ids.front(), world.GetScope()->GetId());
  EXPECT_THAT(scope_ids, ::testing::Contains(std::string{"plane.alpha"}));
  EXPECT_THAT(scope_ids, ::testing::Contains(std::string{"civ.alpha"}));
  EXPECT_THAT(scope_ids, ::testing::Contains(std::string{"city.alpha"}));
}

}  // namespace hs::terra
