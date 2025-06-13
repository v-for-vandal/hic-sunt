#include "region.hpp"

#include <core/utils/serialize.hpp>
#include <core/types/std_base_types.hpp>

#include <gtest/gtest.h>

namespace hs::region {

using namespace ::hs::geometry::literals;

using StdRegion = Region<StdBaseTypes>

TEST(StdRegion, Serialize) {
  StdRegion ref_region(15);

  std::string storage;

  proto::region::StdRegion proto_region;
  SerializeTo(ref_region, proto_region);
  proto_region.SerializeToString(&storage);


  proto::region::StdRegion proto_read_region;
  ASSERT_TRUE(proto_read_region.ParseFromString(storage));

  auto parse_region = ParseFrom(proto_read_region, serialize::To<StdRegion>{});

  EXPECT_EQ(ref_region, parse_region);
  EXPECT_EQ(ref_region.GetSurfaceObject().data_size(), parse_region.GetSurfaceObject().data_size());
}

TEST(StdRegion, SerializeDefaultConstructed) {
  StdRegion ref_region;

  std::string storage;

  proto::region::StdRegion proto_region;
  SerializeTo(ref_region, proto_region);
  proto_region.SerializeToString(&storage);


  proto::region::StdRegion proto_read_region;
  ASSERT_TRUE(proto_read_region.ParseFromString(storage));

  auto parse_region = ParseFrom(proto_read_region, serialize::To<StdRegion>{});

  EXPECT_EQ(ref_region, parse_region);
  EXPECT_EQ(ref_region.GetSurfaceObject().data_size(), parse_region.GetSurfaceObject().data_size());
}

TEST(StdRegion, Equality) {
  StdRegion ref_region;
  EXPECT_EQ(ref_region, ref_region);

  StdRegion test_region;
  test_region.SetId(ref_region.GetId());
  EXPECT_EQ(ref_region, test_region);
}

}
