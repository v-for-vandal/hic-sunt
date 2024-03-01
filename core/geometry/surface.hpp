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
  SurfaceView(CellsArrayView<Cell> target, typename ViewCoords::QAxis q_start, typename ViewCoords::RAxis r_start, typename ViewCoords::SAxis s_start, typename ViewCoords::SAxis s_end):
    target_(std::move(target)),
    q_start_(q_start),
    r_start_(r_start),
    s_start_(s_start),
    s_end_(s_end) {}

  SurfaceView(const SurfaceView&) = default;
  SurfaceView(SurfaceView&&) = default;
  SurfaceView& operator=(const SurfaceView&) = default;
  SurfaceView& operator=(SurfaceView&&) = default;


  Cell& GetCell(ViewCoords coords) {
    return const_cast<Cell&>(const_cast<const SurfaceView&>(*this).GetCell(coords));
  }
  Cell& GetCell(typename ViewCoords::QAxis q, typename ViewCoords::RAxis r) {
    return GetCell(ViewCoords{q,r});
  }

  const Cell& GetCell(ViewCoords coords) const {
    return target_(
      (coords.q()-q_start()).ToUnderlying(),
      (coords.r() - r_start()).ToUnderlying());
  }
  const Cell& GetCell(typename ViewCoords::QAxis q, typename ViewCoords::RAxis r) const {
    return GetCell(ViewCoords{q,r});
  }

  auto q_size() const { return typename CoordinateSystem::QDelta{target_.extent(0)}; }
  auto r_size() const { return typename CoordinateSystem::RDelta{target_.extent(1)}; }

  auto q_end() const { return q_start() + q_size(); }
  auto r_end() const { return r_start() + r_size(); }
  auto s_end() const { return s_end_; }

  auto q_start() const { return q_start_; }
  auto r_start() const { return r_start_; }
  auto s_start() const { return s_start_; }

  bool Contains(ViewCoords coords) const {
    /*spdlog::info("Checking {} vs {}-{},{}-{}, {}-{}",
      coords,
      q_start(), q_end(), r_start(), r_end(), s_start(), s_end()
      );*/
    return
      CheckInRange(coords.q(), q_start(), q_end()) &&
      CheckInRange(coords.r(), r_start(), r_end()) &&
      CheckInRange(coords.s(), s_start(), s_end())
      ;
  }
  bool Contains(typename ViewCoords::QAxis q, typename ViewCoords::RAxis r) const {
    return Contains(ViewCoords{q,r});
  }

  bool operator==(const SurfaceView& other) const;
  bool operator!=(const SurfaceView& other) const {
    return !(*this == other);
  }

  /*
  auto begin() { return target_.begin(); }
  auto end() { return target_.end(); }
  auto begin() const { return target_.begin(); }
  auto end() const { return target_.end(); }
  */

  private:
  static bool CheckInRange(auto x, auto start, auto end) {
    return (x.ToUnderlying() - start.ToUnderlying()) * (end.ToUnderlying() - 1 - x.ToUnderlying()) >= 0;
  }

private:
  CellsArrayView<Cell> target_;
  // we store s_start and s_end explicitly because it is virtual. It limits what cells
  // are in surface, but it doesn't affect the storage itself.
  typename ViewCoords::QAxis q_start_{0};
  typename ViewCoords::RAxis r_start_{0};
  typename ViewCoords::SAxis s_start_{0};
  typename ViewCoords::SAxis s_end_{1}; // not inclusive

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

  explicit Surface(
    typename SCS::QAxis q_start = typename SCS::QAxis{0},
    typename SCS::QAxis q_end = typename SCS::QAxis{1},
    typename SCS::RAxis r_start = typename SCS::RAxis{0},
    typename SCS::RAxis r_end = typename SCS::RAxis{1},
    typename SCS::SAxis s_start = typename SCS::SAxis{0},
    typename SCS::SAxis s_end = typename SCS::SAxis{1}
    );
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
  //bool Contains(ViewCoords coords) const { return cells_.Contains(coords); }

  auto q_size() const { return cells_.q_size(); }
  auto r_size() const { return cells_.r_size(); }
  auto s_size() const { return cells_.s_size(); }

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
  WRLD_REDIRECT_VIEW_CONST_FUNCTION(Contains)
  WRLD_REDIRECT_VIEW_FUNCTION(GetCell)

  WRLD_REDIRECT_VIEW_CONST_FUNCTION(q_size)
  WRLD_REDIRECT_VIEW_CONST_FUNCTION(r_size)
  WRLD_REDIRECT_VIEW_CONST_FUNCTION(s_size)
  WRLD_REDIRECT_VIEW_CONST_FUNCTION(q_start)
  WRLD_REDIRECT_VIEW_CONST_FUNCTION(r_start)
  WRLD_REDIRECT_VIEW_CONST_FUNCTION(s_start)
  WRLD_REDIRECT_VIEW_CONST_FUNCTION(q_end)
  WRLD_REDIRECT_VIEW_CONST_FUNCTION(r_end)
  WRLD_REDIRECT_VIEW_CONST_FUNCTION(s_end)

#undef WRLD_REDIRECT_VIEW_FUNCTION
#undef WRLD_REDIRECT_VIEW_CONST_FUNCTION

  bool operator==(const Surface& other) const {
    return view() == other.view();
  }
  bool operator!=(const Surface& other) const {
    return !(*this == other);
  }

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
  template<typename T>
  static int make_size_sane(T start, T end) {
    if( start > end) {
      std::swap(start, end);
    }

    return (end - start).ToUnderlying();
  }
};

}

#include "surface.inl"
