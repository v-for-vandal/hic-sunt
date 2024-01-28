#include "image.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string_view>

namespace utils {

TEST(Image, Basic) {
  auto resource_prefix = std::getenv("TEST_RESOURCES_DIR");
  ASSERT_NE(nullptr, resource_prefix);

  auto resource_path = std::filesystem::path{std::string_view{resource_prefix}};

  auto image_path = resource_path / "unit-test-img-1.png";

  auto target = Image::LoadImage(image_path.c_str());

  ASSERT_TRUE(target.IsValid());

  EXPECT_EQ(target.width(), 2);
  EXPECT_EQ(target.height(), 2);

  // red, green, blue
  EXPECT_EQ(target.get(0, 0, 0), 255);
  EXPECT_EQ(target.get(0, 0, 1), 0);
  EXPECT_EQ(target.get(0, 0, 1), 0);

  auto equal_fn = [](const auto& array1, const auto& array2) {
    if (array1.extent(0) != array2.size()) {
      return false;
    }
    for (size_t i = 0; i < array2.size(); ++i) {
      if (array1(i) != array2[i]) {
        return false;
      }
    }

    return true;
  };

  EXPECT_TRUE(equal_fn(target.get(0, 0), std::array{255, 0, 0, 255}));
  EXPECT_TRUE(equal_fn(target.get(0, 1), std::array{0, 255, 0, 255}));
  EXPECT_TRUE(equal_fn(target.get(1, 0), std::array{0, 0, 255, 255}));
  EXPECT_TRUE(equal_fn(target.get(1, 1), std::array{0, 0, 0, 255}));
}

}  // namespace utils
