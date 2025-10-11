#include <gtest/gtest.h>

#include <core/utils/minmax.hpp>

TEST(MinMax, BasicInt) {

  hs::utils::MinMax<int> target;
  target.Account(-10);

  EXPECT_EQ(target.GetMin(), -10);
  EXPECT_EQ(target.GetMax(), -10);

  target.Account(4);
  EXPECT_EQ(target.GetMin(), -10);
  EXPECT_EQ(target.GetMax(), 4);

  target.Account(-20);
  EXPECT_EQ(target.GetMin(), -20);
  EXPECT_EQ(target.GetMax(), 4);
}

TEST(MinMax, BasicFloat) {

  hs::utils::MinMax<float> target;
  target.Account(-10);

  EXPECT_EQ(target.GetMin(), -10);
  EXPECT_EQ(target.GetMax(), -10);

  target.Account(4);
  EXPECT_EQ(target.GetMin(), -10);
  EXPECT_EQ(target.GetMax(), 4);

  target.Account(-20);
  EXPECT_EQ(target.GetMin(), -20);
  EXPECT_EQ(target.GetMax(), 4);
}
