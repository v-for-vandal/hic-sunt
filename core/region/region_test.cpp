#include "region.hpp"

#include <gtest/gtest.h>

#include <core/types/std_base_types.hpp>
#include <core/utils/serialize.hpp>

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
  EXPECT_EQ(result, 3.0);  // add=1.0 * mult=(1+2.0)
}

TEST(StdRegion, CellScopeParent) {
  StdRegion region("test", 2);
  ASSERT_NE(region.GetScope(), nullptr);

  region.GetSurface().Foreach([&region](auto, auto& cell) {
    ASSERT_EQ(cell.GetScope()->GetParent(), region.GetScope());
  });
}

TEST(StdRegion, TopNStringValues) {
  StdRegion region("test", 2);
  ASSERT_NE(region.GetScope(), nullptr);

  const char* var_name = "string_variable";

  const std::string FIRST{"first"};
  const std::string SECOND{"second"};
  const std::string THIRD{"third"};
  const std::string FOURTH{"fourth"};

  std::vector<std::string> data;
  std::vector<std::string> first{4, FIRST};
  std::vector<std::string> second{
      4, SECOND};  // same number, but greater in lexicographic score
  std::vector<std::string> third{2, THIRD};
  std::vector<std::string> fourth{1, FOURTH};

  data.insert(data.end(), first.begin(), first.end());
  data.insert(data.end(), second.begin(), second.end());
  data.insert(data.end(), third.begin(), third.end());
  data.insert(data.end(), fourth.begin(), fourth.end());

  // First, assign some values
  region.GetSurface().Foreach([&var_name, &data](auto, auto& cell) mutable {
    ASSERT_NE(cell.GetScope(), nullptr);
    if (data.size()) {
      auto elem = data.back();
      data.pop_back();
      cell.GetScope()->AddStringModifier(var_name, "mod_name", elem, 100);
    };
  });

  ASSERT_TRUE(data.empty());  // make sure every element was used

  auto top1 = region.GetTopNStringValues(var_name, 1);
  ASSERT_FALSE(top1.empty());
  EXPECT_EQ(top1[0],
            make_pair(4, SECOND));  // "second" is lexicographicaly greater

  auto top2 = region.GetTopNStringValues(var_name, 2);
  ASSERT_FALSE(top2.empty());
  EXPECT_EQ(top2[0], make_pair(4, SECOND));
  EXPECT_EQ(top2[1], make_pair(4, FIRST));

  auto top3 = region.GetTopNStringValues(var_name, 3);
  ASSERT_FALSE(top3.empty());
  EXPECT_EQ(top3[0], make_pair(4, SECOND));
  EXPECT_EQ(top3[1], make_pair(4, FIRST));
  EXPECT_EQ(top3[2], make_pair(2, THIRD));

  // We request 5 elements but there are only for non-empty values in the
  // region
  auto top5 = region.GetTopNStringValues(var_name, 5);
  EXPECT_EQ(top5.size(), 4);  // Not 5!
}

}  // namespace hs::region
