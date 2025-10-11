#include "region.hpp"

#include <core/types/std_base_types.hpp>
#include <core/utils/serialize.hpp>

#include <gtest/gtest.h>

namespace hs::region {

using namespace ::hs::geometry::literals;

using StdRegion = Region<StdBaseTypes>;

TEST(StdRegion, Serialize) {
  StdRegion ref_region("test", 15);

  std::string storage;

  proto::region::Region proto_region;
  SerializeTo(ref_region, proto_region);
  proto_region.SerializeToString(&storage);

  proto::region::Region proto_read_region;
  ASSERT_TRUE(proto_read_region.ParseFromString(storage));

  auto parse_region = ParseFrom(proto_read_region, serialize::To<StdRegion>{});

  EXPECT_EQ(ref_region, parse_region);
  EXPECT_EQ(ref_region.GetSurfaceObject().data_size(),
            parse_region.GetSurfaceObject().data_size());
}

TEST(StdRegion, SerializeDefaultConstructed) {
  StdRegion ref_region("test", 4);

  std::string storage;

  proto::region::Region proto_region;
  SerializeTo(ref_region, proto_region);
  proto_region.SerializeToString(&storage);

  proto::region::Region proto_read_region;
  ASSERT_TRUE(proto_read_region.ParseFromString(storage));

  auto parse_region = ParseFrom(proto_read_region, serialize::To<StdRegion>{});

  EXPECT_EQ(ref_region, parse_region);
  EXPECT_EQ(ref_region.GetSurfaceObject().data_size(),
            parse_region.GetSurfaceObject().data_size());
}

TEST(StdRegion, Equality) {
  StdRegion ref_region("test", 2);
  EXPECT_EQ(ref_region, ref_region);

  StdRegion test_region("test", 2);
  EXPECT_EQ(ref_region, test_region);
}

TEST(StdRegion, Scope) {
  StdRegion ref_region("test", 2);
  ASSERT_NE(ref_region.GetScope(), nullptr);

  ref_region.GetScope()->AddNumericModifier("some_var", "some_key", 1.0, 2.0);
  auto result = ref_region.GetScope()->GetNumericValue("some_var");
  EXPECT_EQ(result, 3.0); // add=1.0 * mult=(1+2.0)
}

TEST(StdRegion, CellScopeParent) {
  StdRegion region("test", 2);
  ASSERT_NE(region.GetScope(), nullptr);

  region.GetSurface().foreach(
    [&region](auto, auto& cell) {
    ASSERT_EQ(cell.GetScope()->GetParent(), region.GetScope());
    }
    );
}

} // namespace hs::region
