#include "coords.hpp"
#include "coord_system.hpp"

#include <gtest/gtest.h>

namespace hs::geometry {

TEST(Coords, Comparing) {
  using namespace literals;
  using RawCoords = Coords<QRSCoordinateSystem>;

  EXPECT_EQ((RawCoords{1_q, 2_r}), (RawCoords{1_q, 2_r}));

  EXPECT_LT((RawCoords{1_q, 2_r}), (RawCoords{4_q, 5_r}));
  EXPECT_LE((RawCoords{1_q, 2_r}), (RawCoords{4_q, 5_r}));

  EXPECT_GT((RawCoords{4_q, 5_r}), (RawCoords{1_q, 2_r}));

  EXPECT_FALSE((RawCoords{4_q, 5_r}) < (RawCoords{5_q, 4_r}));
}

}  // namespace terra
