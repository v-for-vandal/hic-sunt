#pragma once

#include <core/geometry/coords.hpp>
#include <core/geometry/surface_raw_view.hpp>
#include <core/utils/serialize.hpp>

#include <mdspan>

namespace hs::geometry {


// Hexagon is centered on (0,0,0)
template<typename CoordinateSystem>
class HexagonSurface {
public:
      using Cell = Cell_;
      using Coords = geometry::Coords<CoordinateSystem>;
      using QDelta = CoordinateSystem::QDelta;
      using RDelta = CoordinateSystem::RDelta;
    HexagonSurface(int radius) {
        if(radius < 1) {
            radius = 1;
        }

        radius_ = radius;
    };

    size_t GetAllocationCount() const noexcept {
        return (radius + radius + 1) * (radius + radius + 1);
    }

      bool Contains(Coords coords) const noexcept {

          return abs(coords.q().ToUnderlying()) + abs(coords.r().ToUnderlying()) <=
              radius;
     }

private:
    int radius_{1};

