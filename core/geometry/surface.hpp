#pragma once

#include <core/geometry/coords.hpp>

#include <mdspan>

namespace geometry {

template<typename Cell>
using CellsArrayView =
    std::mdspan<Cell,
                  std::extents<int, std::dynamic_extent,
                                 std::dynamic_extent, std::dynamic_extent>,
                  std::layout_left>;

template<typename Cell, typename CoordinateSystem>
class SurfaceView {
  using ViewCoords = geometry::Coords<CoordinateSystem>;

  SurfaceView() = default;
  SurfaceView(CellsArrayView<Cell> target):
    target_(std::move(target)) {}

  SurfaceView(const SurfaceView&) = default;
  SurfaceView(SurfaceView&&) = default;
  SurfaceView& operator=(const SurfaceView&) = default;
  SurfaceView& operator=(SurfaceView&&) = default;


  Cell& GetCell(ViewCoords coords) {
    return target(coords.q(), coords.r());
  }

  const Cell& GetCell(ViewCoords coords) const {
    return target(coords.q(), coords.r());
  }

private:
  CellsArrayView<Cell> target_;
};

template<typename Cell, typename CoordinateSystem>
class Surface {

  using View = SurfaceView<Cell, CoordinateSystem>;
  using ConstView = SurfaceView<const Cell, CoordinateSystem>;
  using SCS = CoordinateSystem;

  Surface(typename SCS::QDelta q_size, typename SCS::RDelta r_size);

  View view() { return cells_; }

#define WRLD_REDIRECT_VIEW_CONST_FUNCTION(func)      \
  template <typename... Args>                        \
  decltype(auto) func(Args... args) const {          \
    return cells_.func(std::forward<Args>(args)...); \
  }

#define WRLD_REDIRECT_VIEW_FUNCTION(func)            \
  template <typename... Args>                        \
  decltype(auto) func(Args... args) {                \
    return cells_.func(std::forward<Args>(args)...); \
  }

  WRLD_REDIRECT_VIEW_CONST_FUNCTION(GetCell)

#undef WRLD_REDIRECT_VIEW_FUNCTION
#undef WRLD_REDIRECT_VIEW_CONST_FUNCTION

private:
  const size_t data_size_;
  std::unique_ptr<Cell[]> data_storage_;
  SurfaceView<Cell, CoordinateSystem> cells_;
};

}

#include "surface.inl"
