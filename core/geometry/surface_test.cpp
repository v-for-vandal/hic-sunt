#include "surface.hpp"

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

}
