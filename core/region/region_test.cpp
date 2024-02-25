#include "region.hpp"

#include <core/utils/serialize.hpp>

#include <gtest/gtest.h>

namespace hs::region {

using namespace ::hs::geometry::literals;

TEST(Region, Serialize) {
  Region ref_region(15);

  std::string storage;

  proto::region::Region proto_region;
  SerializeTo(ref_region, proto_region);
  proto_region.SerializeToString(&storage);


  proto::region::Region proto_read_region;
  ASSERT_TRUE(proto_read_region.ParseFromString(storage));

  auto parse_region = ParseFrom(proto_read_region, serialize::To<Region>{});

  EXPECT_EQ(ref_region, parse_region);
  EXPECT_EQ(ref_region.GetSurfaceObject().data_size(), parse_region.GetSurfaceObject().data_size());
}

TEST(Region, SerializeDefaultConstructed) {
  Region ref_region;

  std::string storage;

  proto::region::Region proto_region;
  SerializeTo(ref_region, proto_region);
  proto_region.SerializeToString(&storage);


  proto::region::Region proto_read_region;
  ASSERT_TRUE(proto_read_region.ParseFromString(storage));

  auto parse_region = ParseFrom(proto_read_region, serialize::To<Region>{});

  EXPECT_EQ(ref_region, parse_region);
  EXPECT_EQ(ref_region.GetSurfaceObject().data_size(), parse_region.GetSurfaceObject().data_size());
}

TEST(Region, Equality) {
  Region ref_region;
  EXPECT_EQ(ref_region, ref_region);

  Region test_region;
  test_region.SetId(ref_region.GetId());
  EXPECT_EQ(ref_region, test_region);
}

}
