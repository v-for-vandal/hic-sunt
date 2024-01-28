#include "image.hpp"

namespace utils {

Image Image::LoadImage(const char* filename) {
  int x{0};
  int y{0};
  int channels{0};

  DataPtr data_ptr{stbi_load(filename, &x, &y, &channels, 0)};
  const unsigned char* raw = data_ptr.get();

  return Image{std::move(data_ptr), ImageView(raw, x, y, channels)};
}

}  // namespace utils
