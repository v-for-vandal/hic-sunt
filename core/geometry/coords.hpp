#pragma once

#include <fmt/format.h>

#include <core/utils/assert.hpp>
#include <core/geometry/axis.hpp>
#include <core/geometry/coord_system.hpp>
#include <core/geometry/direction.hpp>

namespace geometry {

namespace details {


  template<typename QAxis, typename RAxis, typename SAxis>
  class QRSCompact {
  public:
    QRSCompact(QAxis q, RAxis r):
      q_(q), r_(r) {}

    QAxis q() const noexcept { return q_; }
    RAxis r() const noexcept { return r_; }
    SAxis s() const noexcept { return SAxis{0 - q.ToUnderlying() - r.ToUnderlying()}; }

    bool operator==(const QRSCompact&) const noexcept = default;

    QAxis q_{0};
    RAxis r_{0};
  };

} // namespace details


/* Axial coordinate system */
template <typename CoordinateSystem>
class DeltaCoords : {
public:
  using QDelta = typename CoordinateSystem::QDelta;
  using RDelta = typename CoordinateSystem::RDelta;
  using SDelta = typename CoordinateSystem::SDelta;

  DeltaCoords(QDelta q, RDelta r):
    data_(q,r) {}

  QDelta q() const noexcept { return data_.q(); }
  RDelta r() const noexcept { return data_.r(); }
  SDelta s() const noexcept { return data_.s(); }

  bool operator==(const DeltaCoords&) const noexcept = default;

  static DeltaCoords GetUndefinedDelta();

private:
  QRSCompact<QDelta, RDelta, SDelta
    > data_;
};

template <typename CoordinateSystem>
class Coords {
public:
  using DeltaCoords = ::geometry::DeltaCoords<CoordinateSystem>;
  using QAxis = typename CoordinateSystem::QAxis;
  using RAxis = typename CoordinateSystem::RAxis;
  using SAxis = typename CoordinateSystem::SAxis;


  Coords(QAxir q, RAxis r):
    data_(q,r) {}

  bool IsUndefined() const noexcept;
  void SetUndefined() noexcept;

  bool operator==(const Coords&) const noexcept = default;

#define COORDS_CMP(op)                                   \
  bool operator op(const Coords& other) const noexcept { \
    return q() op other.q() && r() op other.r() && s() op other.s(); \
  }

  COORDS_CMP(<)
  COORDS_CMP(<=)
  COORDS_CMP(>)
  COORDS_CMP(>=)

  Coords operator+(const DeltaCoords& second) const noexcept;
  DeltaCoords operator-(const Coords& second) const noexcept;

private:
  QRSCompact<QDelta, RDelta, SDelta
    > data_;
};

}  // namespace geometry

template <typename T>
struct fmt::formatter<geometry::Coords<T>> {
  constexpr auto parse(format_parse_context& ctx) const {
    auto it = ctx.begin(), end = ctx.end();
    // Check if reached the end of the range:
    if (it != end && *it != '}') {
      throw fmt::format_error("Invalid format for Coords");
    }

    return it;
  }

  template <typename FormatCtx>
  auto format(const geometry::Coords<T>& coords, FormatCtx& ctx) {
    return fmt::format_to(ctx.out(), "({},{},{})", coords.q().ToUnderlying(),
                          coords.r().ToUnderlying(), coords.s().ToUnderlying());
  }
};

#include "coords.inl"
