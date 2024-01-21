#pragma once

#include <core/geometry/surface.hpp>
#include <core/terra/cell.hpp>

#include <flatbuffers/flatbuffers.h>
#include <fbs/world_generated.h>

namespace hs::terra {

class World {
public:
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using Surface = geometry::Surface<Cell, QRSCoordinateSystem>;
  using SurfaceView = geometry::Surface<Cell, QRSCoordinateSystem>;

  World(QRSCoordinateSystem::QDelta q_size, QRSCoordinateSystem::RDelta r_size);

  const SurfaceView& GetSurface() const { return surface_; }

private:
  Surface surface_;
};



}
