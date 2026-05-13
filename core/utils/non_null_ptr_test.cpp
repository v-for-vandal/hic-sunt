#include <gtest/gtest.h>

#include <core/utils/non_null_ptr.hpp>

namespace hs::utils {

TEST(NonNullSharedPtr, Basic) {
  NonNullSharedPtr<int> ptr;

  ASSERT_NE(ptr, nullptr);

  *ptr = 5;

  ASSERT_EQ(*ptr, 5);

  NonNullSharedPtr<int> ptr2(std::move(ptr));

  ASSERT_NE(ptr, nullptr);
  ASSERT_NE(ptr2, nullptr);
  ASSERT_EQ(*ptr2, 5);

  NonNullSharedPtr<int> ptr3;
  ASSERT_NE(ptr3, nullptr);

  ptr3 = std::move(ptr2);
  ASSERT_NE(ptr, nullptr);
  ASSERT_NE(ptr2, nullptr);
  ASSERT_NE(ptr3, nullptr);
  ASSERT_EQ(*ptr3, 5);
}

TEST(NonNullSharedPtr, ConstAssignment) {
  NonNullSharedPtr<const int> recv(10);
  ASSERT_EQ(10, *recv);
  NonNullSharedPtr<int> val;
  *val = 15;
  ASSERT_EQ(15, *val);

  recv = val;

  ASSERT_EQ(15, *recv);
}

}  // namespace hs::utils
