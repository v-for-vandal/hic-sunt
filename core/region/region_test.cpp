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

} // namespace hs::region
