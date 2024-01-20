#pragma once

#include <fmt/format.h>

#include <array>
#include <cstdint>
#include <string_view>

namespace geometry {

enum class Axis : uint_fast8_t {
  kBegin,
  kX = kBegin,
  kY,
  kZ,

  kSize
};

constexpr auto AllAxes() noexcept {
  constexpr const auto all_axis_ = std::array{Axis::kX, Axis::kY, Axis::kZ};
  return all_axis_;
}

inline constexpr std::string_view ToString(Axis axis) {
  using namespace std::literals;
  switch (axis) {
    case Axis::kX:
      return "X"sv;
    case Axis::kY:
      return "Y"sv;
    case Axis::kZ:
      return "Z"sv;
    default:
      return "UB"sv;
  }
}
}  // namespace terra

template <>
struct fmt::formatter<geometry::Axis> {
  constexpr auto parse(format_parse_context& ctx) const
      -> decltype(ctx.begin()) {
    auto it = ctx.begin(), end = ctx.end();
    // Check if reached the end of the range:
    if (it != end && *it != '}') {
      throw fmt::format_error("Invalid format for Axis");
    }

    return it;
  }

  template <typename FormatCtx>
  auto format(geometry::Axis axis, FormatCtx& ctx) {
    // for some reason, constexpr doesn't work here
    return fmt::format_to(ctx.out(), fmt::runtime(ToString(axis)));
  }
};
