#pragma once

#include <geometry/surface_shape.pb.h>

#include <core/geometry/box.hpp>
#include <core/geometry/coords.hpp>
#include <core/utils/serialize.hpp>
#include <mdspan>

namespace hs::geometry {

class HexagonSurface;
class RhombusSurface;

template <typename CoordinateSystem>
class SurfaceShape;

HexagonSurface ParseFrom(const proto::geometry::HexagonSurface &source,
                         serialize::To<HexagonSurface>);

void SerializeTo(const HexagonSurface &source,
                 proto::geometry::HexagonSurface &target);

// Hexagon is centered on (0,0,0)
class HexagonSurface {
 public:
  using CoordinateSystem = geometry::QRSCoordinateSystem;
  using Coords = geometry::Coords<CoordinateSystem>;
  using Box = geometry::Box<CoordinateSystem>;
  using QDelta = CoordinateSystem::QDelta;
  using RDelta = CoordinateSystem::RDelta;
  using QAxis = typename CoordinateSystem::QAxis;
  using RAxis = typename CoordinateSystem::RAxis;
  HexagonSurface(int radius) {
    if (radius < 1) {
      radius = 1;
    }

    radius_ = radius;
    bounding_box_ = Box{Coords{QAxis{-radius}, RAxis{-radius}},
                        Coords{QAxis{radius}, RAxis{radius}}};
  };

  // If you iterate over all elements in a box, while checking for
  // Contains, you will iterate over all Cells in a surface.
  // This box is also used to allocate data.
  Box BoundingBox() const noexcept { return bounding_box_; }

  bool Contains(Coords coords) const noexcept {
    using std::abs;

    return abs(coords.q()).ToUnderlying() <= radius_ &&
           abs(coords.r()).ToUnderlying() <= radius_ &&
           abs(coords.s()).ToUnderlying() <= radius_;
  }

  bool operator==(const HexagonSurface &) const = default;
  bool operator!=(const HexagonSurface &) const = default;

 private:
  friend HexagonSurface ParseFrom(const proto::geometry::HexagonSurface &source,
                                  serialize::To<HexagonSurface>);

  friend void SerializeTo(const HexagonSurface &source,
                          proto::geometry::HexagonSurface &target);

 private:
  int radius_{1};
  Box bounding_box_{Coords{QAxis{-1}, RAxis{-1}}, Coords{QAxis{2}, RAxis{2}}};
};

RhombusSurface ParseFrom(const proto::geometry::RhombusSurface &source,
                         serialize::To<RhombusSurface>);

void SerializeTo(const RhombusSurface &source,
                 proto::geometry::RhombusSurface &target);
// Uses all available cells in the box. Shape is a bit weird, this class is
// used mostly as placeholder
class RhombusSurface {
 public:
  using CoordinateSystem = geometry::QRSCoordinateSystem;
  using Coords = geometry::Coords<CoordinateSystem>;
  using Box = geometry::Box<CoordinateSystem>;
  using QDelta = CoordinateSystem::QDelta;
  using RDelta = CoordinateSystem::RDelta;
  using QAxis = typename CoordinateSystem::QAxis;
  using RAxis = typename CoordinateSystem::RAxis;
  RhombusSurface(const Box &box) : bounding_box_(box) {};

  // If you iterate over all elements in a box, while checking for
  // Contains, you will iterate over all Cells in a surface.
  // This box is also used to allocate data.
  Box BoundingBox() const noexcept { return bounding_box_; }

  bool Contains(Coords coords) const noexcept {
    return bounding_box_.Contains(coords);
  }

  bool operator==(const RhombusSurface &) const = default;
  bool operator!=(const RhombusSurface &) const = default;

 private:
  friend RhombusSurface ParseFrom(const proto::geometry::RhombusSurface &source,
                                  serialize::To<RhombusSurface>);

  friend void SerializeTo(const RhombusSurface &source,
                          proto::geometry::RhombusSurface &target);

 private:
  Box bounding_box_{Coords{QAxis{-1}, RAxis{-1}}, Coords{QAxis{2}, RAxis{2}}};
};

// This class is not defined because surface shape is highly dependent on
// coordinate system
template <typename CoordinateSystem>
class SurfaceShape;

SurfaceShape<geometry::QRSCoordinateSystem> ParseFrom(
    const proto::geometry::SurfaceShape &source,
    serialize::To<SurfaceShape<geometry::QRSCoordinateSystem>>);
void SerializeTo(const SurfaceShape<geometry::QRSCoordinateSystem> &source,
                 proto::geometry::SurfaceShape &target);

// Implementation of SurfaceShape for QRSCoordinateSystem
template <>
class SurfaceShape<geometry::QRSCoordinateSystem> {
 public:
  using CoordinateSystem = geometry::QRSCoordinateSystem;
  using Coords = geometry::Coords<CoordinateSystem>;
  using Box = geometry::Box<CoordinateSystem>;

  SurfaceShape(const HexagonSurface &hexagon) : data_(hexagon) {}

  SurfaceShape(const RhombusSurface &rhombus) : data_(rhombus) {}

  Box BoundingBox() const noexcept {
    return std::visit([](const auto &e) { return e.BoundingBox(); }, data_);
  }

  bool Contains(Coords coords) const noexcept {
    return std::visit([coords](const auto &e) { return e.Contains(coords); },
                      data_);
  }

  bool operator==(const SurfaceShape &) const = default;
  bool operator!=(const SurfaceShape &) const = default;

 private:
  friend SurfaceShape<geometry::QRSCoordinateSystem> ParseFrom(
      const proto::geometry::SurfaceShape &source,
      serialize::To<SurfaceShape<geometry::QRSCoordinateSystem>>);
  friend void SerializeTo(
      const SurfaceShape<geometry::QRSCoordinateSystem> &source,
      proto::geometry::SurfaceShape &target);

 private:
  std::variant<RhombusSurface, HexagonSurface> data_;
};

}  // namespace hs::geometry

#include "surface_shape.inl"
