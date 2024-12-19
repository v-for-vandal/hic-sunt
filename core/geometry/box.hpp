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


