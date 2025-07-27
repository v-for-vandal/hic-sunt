#pragma once

#include <core/geometry/coords.hpp>

namespace hs::geometry {

// A simple bounding box
template <typename CoordinateSystem>
class Box {
public:
  using QAxis = typename CoordinateSystem::QAxis;
  using RAxis = typename CoordinateSystem::RAxis;
  using SAxis = typename CoordinateSystem::SAxis;
  using Coords = typename hs::geometry::Coords<CoordinateSystem>;

  Box(Coords start, Coords end):
      start_(start), end_(end) {}

  const auto& start() const noexcept { return start_; }
  const auto& end() const noexcept { return end_; }

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


