#pragma once

#include <mdspan>
#include <memory>

#include "stb_image.h"

namespace stdex = std::experimental;

namespace utils {

// This is very basic wrapper for image. It is absolutely not intended
// for textures, meshes, sprites and so on.
class Image {
 public:
  auto get(int x, int y, int channel) const { return data_ref_(x, y, channel); }

  auto get(int x, int y) const {
    return std::submdspan(data_ref_, x, y, std::full_extent);
  }

  bool Contains(int x, int y) const noexcept {
    return x >= 0 && x < width() && y >= 0 && y < height();
  }

  bool IsValid() const noexcept {
    return data_ != nullptr;
  }

  auto get_as_primitive_gray(int x, int y) const noexcept {
    float val{0};
    int ch_count = std::min(channels(), 3);

    auto point_data = get(x, y);

    for (int ch = 0; ch < ch_count; ++ch) {
      val += point_data(ch);
    }

    return static_cast<int>(val / ch_count);
  }

  int width() const { return data_ref_.extent(0); }
  int height() const { return data_ref_.extent(1); }
  int channels() const { return data_ref_.extent(2); }

  static Image LoadImage(const char* filename);

 private:
  struct DataDeleter {
    void operator()(unsigned char* data) const noexcept {
      stbi_image_free(data);
    }
  };

  using DataPtr = std::unique_ptr<unsigned char, DataDeleter>;
  using ImageView = std::mdspan<
      const unsigned char,
      std::extents<size_t, std::dynamic_extent, std::dynamic_extent,
                     std::dynamic_extent>,
      std::layout_right>;

  Image(DataPtr data, const ImageView& data_ref)
      : data_(std::move(data)), data_ref_(data_ref) {}

  DataPtr data_;
  ImageView data_ref_;
};

}  // namespace utils
