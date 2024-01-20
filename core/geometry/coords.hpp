#pragma once

#include <fmt/format.h>

#include <core/utils/assert.hpp>
#include <core/geometry/axis.hpp>
#include <core/geometry/coord_system.hpp>
#include <core/geometry/direction.hpp>

namespace geometry {

template <typename CoordinateSystem>
struct DeltaCoords {
  using XDelta = typename CoordinateSystem::XDelta;
  using YDelta = typename CoordinateSystem::YDelta;
  using ZDelta = typename CoordinateSystem::ZDelta;
  XDelta x{0};
  YDelta y{0};
  ZDelta z{0};

  bool IsSingleAxis() const noexcept {
    int count{0};
    if (x != 0) count++;
    if (y != 0) count++;
    if (z != 0) count++;

    return count == 1;
  }

  Direction GetSingleDirection() const noexcept {
    CHECK(IsSingleAxis());
    if (x != 0) {
      return std::array{Direction::kXNegative, Direction::kXPositive}[x > 0];
    }
    if (y != 0) {
      return std::array{Direction::kXNegative, Direction::kXPositive}[y > 0];
    }
    if (z != 0) {
      return std::array{Direction::kXNegative, Direction::kXPositive}[z > 0];
    }

    std::unreachable();
    return Direction::kSize;
  }
};

template <typename CoordinateSystem>
struct Coords {
  using DeltaCoords = ::geometry::DeltaCoords<CoordinateSystem>;
  using XAxis = typename CoordinateSystem::XAxis;
  using YAxis = typename CoordinateSystem::YAxis;
  using ZAxis = typename CoordinateSystem::ZAxis;

  XAxis x{std::numeric_limits<int>::max()};
  YAxis y{std::numeric_limits<int>::max()};
  ZAxis z{std::numeric_limits<int>::max()};

  bool IsUndefined() const noexcept;
  void SetUndefined() { *this = Coords{}; }

#define COORDS_CMP(op)                                   \
  bool operator op(const Coords& other) const noexcept { \
    return x op other.x && y op other.y && z op other.z; \
  }

  COORDS_CMP(<)
  COORDS_CMP(<=)
  COORDS_CMP(>)
  COORDS_CMP(>=)

#undef COORDS_CMP

  bool operator==(const Coords&) const noexcept = default;
  bool operator!=(const Coords&) const noexcept = default;

  Coords<RawCoordinateSystem> ToRaw() const;
  // Coords GetNeighbour(Direction ... direction) const noexcept;
  Coords operator+(Direction second) const noexcept;
  Coords operator+(const DeltaCoords& second) const noexcept;
  DeltaCoords operator-(const Coords& second) const noexcept;
};

using RawCoords = Coords<RawCoordinateSystem>;

}  // namespace geometry

template <typename T>
struct fmt::formatter<geometry::Coords<T>> {
  constexpr auto parse(format_parse_context& ctx) const {
    auto it = ctx.begin(), end = ctx.end();
    // Check if reached the end of the range:
    if (it != end && *it != '}') {
      throw fmt::format_error("Invalid format for Coords");
    }

    return it;
  }

  template <typename FormatCtx>
  auto format(const geometry::Coords<T>& coords, FormatCtx& ctx) {
    return fmt::format_to(ctx.out(), "({},{},{})", coords.x.ToUnderlying(),
                          coords.y.ToUnderlying(), coords.z.ToUnderlying());
  }
};

#include "coords.inl"
