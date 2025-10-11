#pragma once

#include <core/geometry/coords.hpp>
#include <core/geometry/surface_raw_view.hpp>
#include <core/utils/serialize.hpp>
#include <mdspan>

namespace hs::geometry {

// SurfaceRectView maps rectangle over SurfaceRawView. All cells that are
// outside of rectangle are considered 'not present', even if they are
// physically in storage
// However, SurfaceRectView still uses qr-coords, not xy-coords. It is just
// Contains() that is changed
template <typename Cell, typename CoordinateSystem>
class SurfaceRectView {
 public:
  using Coords = geometry::Coords<CoordinateSystem>;

  SurfaceRectView() = default;
  SurfaceRectView(SurfaceRawView raw_view) : raw_view_(std::move(raw_view)) {
    // calculate x_start, x_end, y_start, y_end
  }

  SurfaceRectView(const SurfaceRectView &) = default;
  SurfaceRectView(SurfaceRectView &&) = default;
  SurfaceRectView &operator=(const SurfaceRectView &) = default;
  SurfaceRectView &operator=(SurfaceRectView &&) = default;

  Cell &GetCell(Coords coords) {
    return const_cast<Cell &>(
        const_cast<const SurfaceRectView &>(*this).GetCell(coords));
  }
  Cell &GetCell(typename Coords::QAxis q, typename Coords::RAxis r) {
    return GetCell(Coords{q, r});
  }

  const Cell &GetCell(Coords coords) const {
    return target_((coords.q() - q_start()).ToUnderlying(),
                   (coords.r() - r_start()).ToUnderlying());
  }
  const Cell &GetCell(typename Coords::QAxis q,
                      typename Coords::RAxis r) const {
    return GetCell(Coords{q, r});
  }

  auto q_size() const {
    return typename CoordinateSystem::QDelta{target_.extent(0)};
  }
  auto r_size() const {
    return typename CoordinateSystem::RDelta{target_.extent(1)};
  }

  auto q_end() const { return q_start() + q_size(); }
  auto r_end() const { return r_start() + r_size(); }
  auto s_end() const { return s_end_; }

  auto q_start() const { return q_start_; }
  auto r_start() const { return r_start_; }
  auto s_start() const { return s_start_; }

  bool Contains(Coords coords) const {
    /*spdlog::info("Checking {} vs {}-{},{}-{}, {}-{}",
      coords,
      q_start(), q_end(), r_start(), r_end(), s_start(), s_end()
      );*/
    return CheckInRange(coords.q(), q_start(), q_end()) &&
           CheckInRange(coords.r(), r_start(), r_end()) &&
           CheckInRange(coords.s(), s_start(), s_end());
  }
  bool Contains(typename Coords::QAxis q, typename Coords::RAxis r) const {
    return Contains(Coords{q, r});
  }

  bool operator==(const SurfaceRectView &other) const;
  bool operator!=(const SurfaceRectView &other) const {
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
    return (x.ToUnderlying() - start.ToUnderlying()) *
               (end.ToUnderlying() - 1 - x.ToUnderlying()) >=
           0;
  }

 private:
  SurfaceRawView raw_view_;
  // we store s_start and s_end explicitly because it is virtual. It limits what
  // cells are in surface, but it doesn't affect the storage itself.
  typename Coords::QAxis x_start_{0};
  typename Coords::RAxis y_start_{0};
  typename Coords::SAxis x_end{1};
  typename Coords::SAxis y_end_{1};  // not inclusive
};

}  // namespace hs::geometry
