#include "coords.hpp"

#include <gtest/gtest.h>

namespace geometry {

TEST(Coords, Comparing) {
  using namespace literals;
  EXPECT_EQ((RawCoords{1_x, 2_y, 3_z}), (RawCoords{1_x, 2_y, 3_z}));

  EXPECT_LT((RawCoords{1_x, 2_y, 3_z}), (RawCoords{4_x, 5_y, 6_z}));
  EXPECT_LE((RawCoords{1_x, 2_y, 3_z}), (RawCoords{4_x, 5_y, 3_z}));

  EXPECT_GT((RawCoords{4_x, 5_y, 6_z}), (RawCoords{1_x, 2_y, 3_z}));

  EXPECT_FALSE((RawCoords{4_x, 5_y, 6_z}) < (RawCoords{5_x, 4_y, 6_z}));
}

}  // namespace terra
