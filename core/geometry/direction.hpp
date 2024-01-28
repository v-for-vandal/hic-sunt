#pragma once

#include <fmt/format.h>

#include <array>
#include <core/utils/assert.hpp>
#include <core/geometry/axis.hpp>
#include <string_view>

namespace hs::geometry {

enum class Direction : uint_fast8_t {
  kFirst = 0,
  kQNegative = kFirst,
  kQPositive,
  kRNegative,
  kRPositive,
  kSNegative,
  kSPositive,

  kSize
};

inline constexpr std::string_view ToString(Direction direction) {
  using namespace std::literals;
  switch (direction) {
    case Direction::kQNegative:
      return "kQNegative"sv;
    case Direction::kQPositive:
      return "kQPositive"sv;
    case Direction::kRNegative:
      return "kRNegative"sv;
    case Direction::kRPositive:
      return "kRPositive"sv;
    case Direction::kSNegative:
      return "kSNegative"sv;
    case Direction::kSPositive:
      return "kSPositive"sv;
    default:
      CHECK(false);
      return "UB"sv;
  }
}

inline constexpr auto GetAllDirections() noexcept {
  // TODO: ranges!
  constexpr const std::array directions_{
      Direction::kQNegative, Direction::kQPositive, Direction::kRNegative,
      Direction::kRPositive, Direction::kSNegative, Direction::kSPositive};
  return directions_;
}

inline constexpr Direction GetReverseDirection(Direction input) noexcept {
  switch (input) {
    case Direction::kQNegative:
      return Direction::kQPositive;
    case Direction::kQPositive:
      return Direction::kQNegative;
    case Direction::kRNegative:
      return Direction::kRPositive;
    case Direction::kRPositive:
      return Direction::kRNegative;
    case Direction::kSNegative:
      return Direction::kSPositive;
    case Direction::kSPositive:
      return Direction::kSNegative;
    default:
      CHECK(false);
      return Direction::kSize;
  }

  return Direction::kSize;
}

inline constexpr std::array<Direction, 2> GetAxisDirections(
    Axis axis) noexcept {
  switch (axis) {
    case Axis::kQ:
      return std::array{Direction::kQNegative, Direction::kQPositive};
    case Axis::kR:
      return std::array{Direction::kRNegative, Direction::kRPositive};
    case Axis::kS:
      return std::array{Direction::kSNegative, Direction::kSPositive};
    default:
      CHECK(false);
      return std::array{Direction::kSize, Direction::kSize};
  }

  return std::array{Direction::kSize, Direction::kSize};
}

inline Direction GetNegativeDirection(Axis axis) noexcept {
  switch (axis) {
    case Axis::kQ:
      return Direction::kQNegative;
    case Axis::kR:
      return Direction::kRNegative;
    case Axis::kS:
      return Direction::kSNegative;
    default:
      CHECK(false);
      std::unreachable();
  }

  std::unreachable();
  return Direction::kSize;
}

inline Direction GetPositiveDirection(Axis axis) noexcept {
  switch (axis) {
    case Axis::kQ:
      return Direction::kQPositive;
    case Axis::kR:
      return Direction::kRPositive;
    case Axis::kS:
      return Direction::kSPositive;
    default:
      CHECK(false);
      return Direction::kSize;
  }

  return Direction::kSize;
}

inline bool IsPositiveDirection(Direction direction) noexcept {
  switch (direction) {
    case Direction::kQNegative:
    case Direction::kRNegative:
    case Direction::kSNegative:
      return false;
    case Direction::kSPositive:
    case Direction::kQPositive:
    case Direction::kRPositive:
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
struct fmt::formatter<::hs::geometry::Direction> {
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
  auto format(::hs::geometry::Direction direction, FormatCtx& ctx) {
    // for some reason, constexpr doesn't work here
    return fmt::format_to(ctx.out(), fmt::runtime(ToString(direction)));
  }
};
