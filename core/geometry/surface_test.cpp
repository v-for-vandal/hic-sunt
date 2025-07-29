#include "surface.hpp"

#include <absl/container/flat_hash_set.h>

#include <gtest/gtest.h>

namespace hs::geometry{

TEST(Surface, MoveCapable) {

  struct Cell {};
  using SurfaceT = Surface<Cell, QRSCoordinateSystem>;

  {
    SurfaceT source;
    SurfaceT target;
    target = std::move(source);
  }

  {
    SurfaceT source;
    SurfaceT target(std::move(source));
  }

}

TEST(Surface, ForEach) {
  struct Cell {};
  using SurfaceT = Surface<Cell, QRSCoordinateSystem>;
  using CoordsT = typename SurfaceT::Coords;
  using Box = Box<QRSCoordinateSystem>;

  // at the moment of writing, s_coords did not matter
  auto start = CoordsT::MakeCoords(-2, -3);
  auto end = CoordsT::MakeCoords(4, 4);
  SurfaceT target{
      RhombusSurface(
          Box(start, end))
  };

  absl::flat_hash_set<typename SurfaceT::Coords> visited;

  auto visitor = [&visited](auto& coords, auto&) {
      EXPECT_FALSE(visited.contains(coords));
      visited.insert(coords);
  };

  target.view().foreach(visitor);

  EXPECT_EQ(visited.size(), 42);
}

}
