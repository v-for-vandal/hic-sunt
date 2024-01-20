#pragma once

#include <fmt/format.h>

#include <array>
#include <core/utils/assert.hpp>
#include <core/geometry/axis.hpp>
#include <string_view>

namespace geometry {

enum class Direction : uint_fast8_t {
  kFirst = 0,
  kXNegative = kFirst,
  kXPositive,
  kYNegative,
  kYPositive,
  kZNegative,
  kZPositive,

  kSize
};

inline constexpr std::string_view ToString(Direction direction) {
  using namespace std::literals;
  switch (direction) {
    case Direction::kXNegative:
      return "kXNegative"sv;
    case Direction::kXPositive:
      return "kXPositive"sv;
    case Direction::kYNegative:
      return "kYNegative"sv;
    case Direction::kYPositive:
      return "kYPositive"sv;
    case Direction::kZNegative:
      return "kZNegative"sv;
    case Direction::kZPositive:
      return "kZPositive"sv;
    default:
      CHECK(false);
      return "UB"sv;
  }
}

inline constexpr auto GetAllDirections() noexcept {
  // TODO: ranges!
  constexpr const std::array directions_{
      Direction::kXNegative, Direction::kXPositive, Direction::kYNegative,
      Direction::kYPositive, Direction::kZNegative, Direction::kZPositive};
  return directions_;
}

inline constexpr Direction GetReverseDirection(Direction input) noexcept {
  switch (input) {
    case Direction::kXNegative:
      return Direction::kXPositive;
    case Direction::kXPositive:
      return Direction::kXNegative;
    case Direction::kYNegative:
      return Direction::kYPositive;
    case Direction::kYPositive:
      return Direction::kYNegative;
    case Direction::kZNegative:
      return Direction::kZPositive;
    case Direction::kZPositive:
      return Direction::kZNegative;
    default:
      CHECK(false);
      return Direction::kSize;
  }

  return Direction::kSize;
}

inline constexpr std::array<Direction, 2> GetAxisDirections(
    Axis axis) noexcept {
  switch (axis) {
    case Axis::kX:
      return std::array{Direction::kXNegative, Direction::kXPositive};
    case Axis::kY:
      return std::array{Direction::kYNegative, Direction::kYPositive};
    case Axis::kZ:
      return std::array{Direction::kZNegative, Direction::kZPositive};
    default:
      CHECK(false);
      return std::array{Direction::kSize, Direction::kSize};
  }

  return std::array{Direction::kSize, Direction::kSize};
}

inline Direction GetNegativeDirection(Axis axis) noexcept {
  switch (axis) {
    case Axis::kX:
      return Direction::kXNegative;
    case Axis::kY:
      return Direction::kYNegative;
    case Axis::kZ:
      return Direction::kZNegative;
    default:
      CHECK(false);
      std::unreachable();
  }

  std::unreachable();
  return Direction::kSize;
}

inline Direction GetPositiveDirection(Axis axis) noexcept {
  switch (axis) {
    case Axis::kX:
      return Direction::kXPositive;
    case Axis::kY:
      return Direction::kYPositive;
    case Axis::kZ:
      return Direction::kZPositive;
    default:
      CHECK(false);
      return Direction::kSize;
  }

  return Direction::kSize;
}

inline bool IsPositiveDirection(Direction direction) noexcept {
  switch (direction) {
    case Direction::kXNegative:
    case Direction::kYNegative:
    case Direction::kZNegative:
      return false;
    case Direction::kZPositive:
    case Direction::kXPositive:
    case Direction::kYPositive:
      return true;
    default:
      CHECK(false);
      return false;
  }

  return false;
}

inline bool IsNegativeDirection(Direction direction) noexcept {
  return !IsPositiveDirection(direction);
}

}  // namespace terra

template <>
struct fmt::formatter<geometry::Direction> {
  constexpr auto parse(format_parse_context& ctx) const
      -> decltype(ctx.begin()) {
    auto it = ctx.begin(), end = ctx.end();
    // Check if reached the end of the range:
    if (it != end && *it != '}') {
      throw fmt::format_error("Invalid format for Direction");
    }

    return it;
  }

  template <typename FormatCtx>
  auto format(geometry::Direction direction, FormatCtx& ctx) {
    // for some reason, constexpr doesn't work here
    return fmt::format_to(ctx.out(), fmt::runtime(ToString(direction)));
  }
};
