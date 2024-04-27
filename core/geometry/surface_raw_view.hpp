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

// RawView is view over raw surface data. It correspondes to physical presence -
// method Contains returns true if such elemen is in data storage, and false
// otherwise. That is why it has no 'S' coordinate - because we store only
// QR coordinates
// It is not intended to be used directly - instead other views are based on
// this one.
template<typename Cell, typename CoordinateSystem>
class SurfaceRawView {
public:
  using Coords = geometry::Coords<CoordinateSystem>;

  SurfaceRawView() = default;
  SurfaceRawView(CellsArrayView<Cell> target, typename Coords::QAxis q_start, typename Coords::RAxis r_start):
    target_(std::move(target)),
    q_start_(q_start),
    r_start_(r_start)
  {}

  SurfaceRawView(const SurfaceRawView&) = default;
  SurfaceRawView(SurfaceRawView&&) = default;
  SurfaceRawView& operator=(const SurfaceRawView&) = default;
  SurfaceRawView& operator=(SurfaceRawView&&) = default;


  Cell& GetCell(Coords coords) {
    return const_cast<Cell&>(const_cast<const SurfaceRawView&>(*this).GetCell(coords));
  }
  Cell& GetCell(typename Coords::QAxis q, typename Coords::RAxis r) {
    return GetCell(Coords{q,r});
  }

  const Cell& GetCell(Coords coords) const {
    return target_(
      (coords.q()-q_start()).ToUnderlying(),
      (coords.r() - r_start()).ToUnderlying());
  }
  const Cell& GetCell(typename Coords::QAxis q, typename Coords::RAxis r) const {
    return GetCell(Coords{q,r});
  }

  auto q_size() const { return typename CoordinateSystem::QDelta{target_.extent(0)}; }
  auto r_size() const { return typename CoordinateSystem::RDelta{target_.extent(1)}; }

  auto q_end() const { return q_start() + q_size(); }
  auto r_end() const { return r_start() + r_size(); }

  auto q_start() const { return q_start_; }
  auto r_start() const { return r_start_; }

  bool Contains(Coords coords) const {
    /*spdlog::info("Checking {} vs {}-{},{}-{}, {}-{}",
      coords,
      q_start(), q_end(), r_start(), r_end(), s_start(), s_end()
      );*/
    return
      CheckInRange(coords.q(), q_start(), q_end()) &&
      CheckInRange(coords.r(), r_start(), r_end());
      ;
  }
  bool Contains(typename Coords::QAxis q, typename Coords::RAxis r) const {
    return Contains(Coords{q,r});
  }

  bool operator==(const SurfaceRawView& other) const;
  bool operator!=(const SurfaceRawView& other) const {
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
  typename Coords::QAxis q_start_{0};
  typename Coords::RAxis r_start_{0};
};

}
