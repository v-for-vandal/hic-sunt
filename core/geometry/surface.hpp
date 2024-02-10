#pragma once

#include <core/geometry/coords.hpp>
#include <core/utils/serialize.hpp>

#include <mdspan>

namespace hs::geometry {

template<typename Cell>
using CellsArrayView =
    std::mdspan<Cell,
                  std::extents<int, std::dynamic_extent,
                                 std::dynamic_extent>,
                  std::layout_left>;

template<typename Cell, typename CoordinateSystem>
class SurfaceView {
public:
  using ViewCoords = geometry::Coords<CoordinateSystem>;

  SurfaceView() = default;
  SurfaceView(CellsArrayView<Cell> target):
    target_(std::move(target)) {}

  SurfaceView(const SurfaceView&) = default;
  SurfaceView(SurfaceView&&) = default;
  SurfaceView& operator=(const SurfaceView&) = default;
  SurfaceView& operator=(SurfaceView&&) = default;


  Cell& GetCell(ViewCoords coords) {
    return target_(coords.q().ToUnderlying(), coords.r().ToUnderlying());
  }
  Cell& GetCell(typename ViewCoords::QAxis q, typename ViewCoords::RAxis r) {
    return GetCell(ViewCoords{q,r});
  }

  const Cell& GetCell(ViewCoords coords) const {
    return target_(coords.q().ToUnderlying(), coords.r().ToUnderlying());
  }
  const Cell& GetCell(typename ViewCoords::QAxis q, typename ViewCoords::RAxis r) const {
    return GetCell(ViewCoords{q,r});
  }

  auto q_size() const { return typename CoordinateSystem::QDelta{target_.extent(0)}; }
  auto r_size() const { return typename CoordinateSystem::RDelta{target_.extent(1)}; }

  auto q_end() const { return typename CoordinateSystem::QAxis{0} + q_size(); }
  auto r_end() const { return typename CoordinateSystem::RAxis{0} + r_size(); }

  auto q_start() const { return typename CoordinateSystem::QAxis{0}; }
  auto r_start() const { return typename CoordinateSystem::RAxis{0}; }

  bool Contains(ViewCoords coords) const {
    return coords.q() >= q_start() && coords.q() < q_end() && coords.r() >= r_start() && coords.r() < r_end();
  }
  bool Contains(typename ViewCoords::QAxis q, typename ViewCoords::RAxis r) const {
    return Contains(ViewCoords{q,r});
  }

  /*
  auto begin() { return target_.begin(); }
  auto end() { return target_.end(); }
  auto begin() const { return target_.begin(); }
  auto end() const { return target_.end(); }
  */

private:
  CellsArrayView<Cell> target_;
};

template<typename Cell, typename CoordinateSystem>
class Surface;

/* TODO: RM
template<typename Cell, typename CoordinateSystem>
auto SerializeTo(const Surface<Cell, CoordinateSystem>& source, ::flatbuffers::FlatBufferBuilder& fbb, auto to);

template<typename Cell, typename CoordinateSystem>
Surface<Cell, CoordinateSystem> ParseFrom(const auto& fbs_class, serialize::To<Surface<Cell, CoordinateSystem>>);
*/

template<typename Cell, typename CoordinateSystem>
class Surface {
public:

  using View = SurfaceView<Cell, CoordinateSystem>;
  using ViewCoords = geometry::Coords<CoordinateSystem>;
  using ConstView = SurfaceView<const Cell, CoordinateSystem>;
  using SCS = CoordinateSystem;
  using SCSize = DeltaCoords<CoordinateSystem>;

  Surface(typename SCS::QDelta q_size = typename SCS::QDelta{1}, typename SCS::RDelta r_size = typename SCS::RDelta{1});
  explicit Surface(SCSize size);
  Surface(const Surface&) = delete;
  Surface(Surface&&) = default;
  Surface& operator=(const Surface&) = delete;
  Surface& operator=(Surface&&) = default;

  View view() { return cells_; }
  View view() const { return cells_; }

  /*
  auto begin() { return cells_.begin(); }
  auto begin() const { return cells_.begin(); }
  auto end() { return cells_.end(); }
  auto end() const { return cells_.end(); }
  */

  size_t data_size() const { return data_size_; }
  const Cell& GetCell(size_t idx) const { return data_storage_[idx]; }
  Cell& GetCell(size_t idx) { return data_storage_[idx]; }
  bool Contains(ViewCoords coords) const { return cells_.Contains(coords); }

  auto q_size() const { return cells_.q_size(); }
  auto r_size() const { return cells_.r_size(); }

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
  WRLD_REDIRECT_VIEW_FUNCTION(GetCell)

#undef WRLD_REDIRECT_VIEW_FUNCTION
#undef WRLD_REDIRECT_VIEW_CONST_FUNCTION

private:
    /*
  friend Surface<Cell,CoordinateSystem> ParseFrom(const auto& fbs_class,
    serialize::To<Surface<Cell, CoordinateSystem>>);
  friend auto SerializeTo(const Surface<Cell, CoordinateSystem>& source, ::flatbuffers::FlatBufferBuilder& fbb, auto to);
  */

  size_t data_size_;
  std::unique_ptr<Cell[]> data_storage_;
  SurfaceView<Cell, CoordinateSystem> cells_;

private:
  // checks that everything is > 0, if not - makes it equal to 1
  // and logs critical error
  static auto make_size_sane(auto coord_elem) {
    auto result = coord_elem.ToUnderlying();
    if(result <= 0) {
      spdlog::critical("size is <= 0");
      result = 1;
    }

    return result;
  }
};

}

#include "surface.inl"
