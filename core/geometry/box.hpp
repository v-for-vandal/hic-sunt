#pragma once

#include <core/geometry/coords.hpp>
#include <core/utils/assert.hpp>

namespace hs::geometry {

// A simple bounding box. Both points are inclusive!
template <typename CoordinateSystem>
class Box {
public:
  using QAxis = typename CoordinateSystem::QAxis;
  using RAxis = typename CoordinateSystem::RAxis;
  using SAxis = typename CoordinateSystem::SAxis;
  using QDelta = typename CoordinateSystem::QDelta;
  using RDelta = typename CoordinateSystem::RDelta;
  using Coords = typename hs::geometry::Coords<CoordinateSystem>;

  Box() noexcept = default;
  Box(Coords start, Coords end):
    start_(start), end_(end) {
      start_ = Coords(std::min(start.q(), end.q()), std::min(start.r(), end.r()));
      end_ = Coords(std::max(start.q(), end.q()), std::max(start.r(), end.r()));
  }

  const auto& start() const noexcept { return start_; }
  const auto& end() const noexcept { return end_; }

  auto q_size() const noexcept { return abs(end_.q() - start_.q()) + QDelta(1); }
  auto r_size() const noexcept { return abs(end_.r() - start_.r()) + RDelta(1); }

  bool operator==(const Box&) const = default;
  bool operator!=(const Box&) const = default;

  static Box MakeOne() noexcept {
        return Box{
            Coords{},
            Coords{},
        };
    }

  bool Contains(Coords coords) const noexcept {
      return coords.q().InRange(start_.q(), end_.q()) &&
          coords.r().InRange(start_.r(), end_.r()) &&
          coords.s().InRange(start_.s(), end_.s());
  }

private:
  Coords start_;
  Coords end_;
};

}

template <typename T>
struct fmt::formatter<hs::geometry::Box<T>> {
  constexpr auto parse(format_parse_context& ctx) const {
    auto it = ctx.begin(), end = ctx.end();
    // Check if reached the end of the range:
    if (it != end && *it != '}') {
      throw fmt::format_error("Invalid format for Box");
    }

    return it;
  }

  template <typename FormatCtx>
  auto format(const hs::geometry::Box<T>& box, FormatCtx& ctx) {
    return fmt::format_to(ctx.out(), "[{};{})", box.start(),
        box.end());

  }
};


