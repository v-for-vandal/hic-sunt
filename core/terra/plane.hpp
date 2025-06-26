#pragma once

#include <core/geometry/surface.hpp>
#include <core/region/region.hpp>
#include <core/region/types.hpp>
#include <core/terra/cell.hpp>
#include <core/terra/types.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/utils/serialize.hpp>
#include <core/types/std_base_types.hpp>


#include <terra/world.pb.h>

namespace hs::terra {

template<typename BaseTypes>
class Plane;

template<typename BaseTypes>
void SerializeTo(const Plane<BaseTypes>& source, proto::terra::Plane& target);
template<typename BaseTypes>
Plane<BaseTypes> ParseFrom(const proto::terra::Plane& world, serialize::To<Plane<BaseTypes>>);

// Plane is essentially one playable map that consists of multiple regions
template<typename BaseTypes = StdBaseTypes>
class Plane {
public:
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
  using QRSSize = geometry::DeltaCoords<geometry::QRSCoordinateSystem>;
  using QRSBox = geometry::Box<geometry::QRSCoordinateSystem>;
  using Region = region::Region<BaseTypes>;
  using RegionPtr = region::RegionPtr<BaseTypes>;
  using Cell = Cell<BaseTypes>;
  using Surface = geometry::Surface<Cell, QRSCoordinateSystem>;
  using SurfaceView = geometry::SurfaceView<Cell, QRSCoordinateSystem>;
  using StringId = BaseTypes::StringId;
  using String = BaseTypes::String;

  Plane() = default;
  Plane(const Plane&) = delete;
  Plane(Plane&&) = default;
  Plane& operator=(const Plane&) = delete;
  Plane& operator=(Plane&&) = default;

  Plane(
    QRSBox box,
    int region_radius
    );

  const StringId& GetPlaneId() const { return plane_id_; }

  SurfaceView GetSurface() const { return surface_.view(); }
  SurfaceView GetSurface() { return surface_.view(); }

  const Surface& GetSurfaceObject() const { return surface_; }

  RegionPtr GetRegionById(const StringId& region_id) const;
  bool HasRegion(const StringId& region_id) const {
    return region_index_.contains(region_id);
  }
  void SetRegion(QRSCoords coords, Region region);

  bool operator==(const Plane& other) const;
  bool operator!=(const Plane& other) const {
    return !(*this == other);
  }

private:
  friend void SerializeTo<BaseTypes>(const Plane& source, proto::terra::Plane& target);
  friend Plane ParseFrom<BaseTypes>(const proto::terra::Plane& world, serialize::To<Plane>);

  void InitNonpersistent();

private:
  StringId plane_id_;
  Surface surface_;
  // some regions are not really part of the surface. Like caves.
  std::vector<Cell> off_surface_;
  std::unordered_map<StringId, RegionPtr> region_index_;

  // TODO: Make atomic? Or just use uuid generator
  static inline int next_id_{0};
};


}

#include "plane.inl"
