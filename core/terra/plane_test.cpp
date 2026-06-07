#include "plane.hpp"

#include <gtest/gtest.h>

#include <core/utils/serialize.hpp>

#include <algorithm>

namespace hs::terra {

using namespace ::hs::geometry::literals;

using StdPlane = Plane<>;

TEST(StdPlane, VisitScopesVisitsOwnScopeThenAllRegionAndCellScopes) {
  StdPlane plane(
      ControlObjectPtr{}, "plane.alpha",
      StdPlane::QRSBox(StdPlane::QRSCoords(-1_q, -1_r), StdPlane::QRSCoords(1_q, 1_r)), 1);

  std::vector<std::string> scope_ids;
  plane.VisitScopes([&scope_ids](const auto& scope_ptr) {
    ASSERT_NE(scope_ptr, nullptr);
    scope_ids.push_back(std::string{scope_ptr->GetId()});
  });

  size_t region_count = 0;
  size_t cell_count = 0;
  plane.GetSurface().Foreach([&](auto, auto& world_cell) {
    ++region_count;
    world_cell.GetRegion().GetSurface().Foreach([&](auto, auto&) { ++cell_count; });
  });

  ASSERT_EQ(scope_ids.size(), 1 + region_count + cell_count);
  EXPECT_EQ(scope_ids.front(), plane.GetScope()->GetId());
  EXPECT_EQ(std::count(scope_ids.begin(), scope_ids.end(), std::string{plane.GetScope()->GetId()}), 1);
}

TEST(StdPlane, Serialize) {
  StdPlane ref_plane(
      ControlObjectPtr{}, "test",

      StdPlane::QRSBox(StdPlane::QRSCoords(-1_q, -2_r), StdPlane::QRSCoords(4_q, 6_r)), 4);

  std::string storage;

  proto::terra::Plane proto_plane;
  SerializeTo(ref_plane, proto_plane);
  proto_plane.SerializeToString(&storage);

  proto::terra::Plane proto_read_plane;
  ASSERT_TRUE(proto_read_plane.ParseFromString(storage));

  auto parse_plane = ParseFrom(proto_read_plane, serialize::To<StdPlane>{});

  EXPECT_EQ(ref_plane, parse_plane);
  EXPECT_EQ(ref_plane.GetSurfaceObject().data_size(), parse_plane.GetSurfaceObject().data_size());
}

}  // namespace hs::terra
