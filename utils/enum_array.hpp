#pragma once

#include <array>
#include <utility>

namespace utils {

struct fill_tag_t {};
constexpr fill_tag_t fill_tag;


template <typename T, typename E>
class EnumArray : public std::array<T, static_cast<size_t>(E::kSize)> {
  // TODO: use std::to_underlying as soon as it is ready
  static_assert(static_cast<int64_t>(E::kSize) > 0,
                "enum must have at least one element");

 public:
  static constexpr int kSize{E::kSize};
  using Base = std::array<T, static_cast<size_t>(E::kSize)>;
  using Base::Base;
  EnumArray(fill_tag_t, auto value) {
    std::fill(Base::begin(), Base::end(), value);
  }

  using Base::operator[];
  constexpr auto& operator[](E elem) noexcept {
    return this->Base::operator[](static_cast<size_t>(elem));
  }
  constexpr const auto& operator[](E elem) const noexcept {
    return this->Base::operator[](static_cast<size_t>(elem));
  }
};

}  // namespace utils
