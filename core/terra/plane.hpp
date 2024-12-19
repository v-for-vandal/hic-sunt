#pragma once

#include <core/geometry/surface.hpp>
#include <core/region/region.hpp>
#include <core/region/types.hpp>
#include <core/terra/cell.hpp>
#include <core/terra/types.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/utils/serialize.hpp>

#include <flatbuffers/flatbuffers.h>
#include <fbs/world_generated.h>

#include <terra/world.pb.h>

namespace hs::terra {

class Plane;

void SerializeTo(const Plane& source, proto::terra::Plane& target);
Plane ParseFrom(const proto::terra::Plane& world, serialize::To<Plane>);

// Plane is essentially one playable map that consists of multiple regions
class Plane {
public:
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
  using QRSSize = geometry::DeltaCoords<geometry::QRSCoordinateSystem>;
  using QRSBox = geometry::Box<geometry::QRSCoordinateSystem>;
  using Surface = geometry::Surface<Cell, QRSCoordinateSystem>;
  using SurfaceView = geometry::SurfaceView<Cell, QRSCoordinateSystem>;

  Plane() = default;
  Plane(const Plane&) = delete;
  Plane(Plane&&) = default;
  Plane& operator=(const Plane&) = delete;
  Plane& operator=(Plane&&) = default;

  Plane(
    QRSBox box
    );

  PlaneIdCRef GetPlaneId() const { return plane_id_; }

  SurfaceView GetSurface() const { return surface_.view(); }
  SurfaceView GetSurface() { return surface_.view(); }

  const Surface& GetSurfaceObject() const { return surface_; }

  region::RegionPtr GetRegionById(const std::string& region_id) const;
  bool HasRegion(const std::string& region_id) const {
    return region_index_.contains(region_id);
  }
  void SetRegion(QRSCoords coords, region::Region region);

  bool operator==(const Plane& other) const;
  bool operator!=(const Plane& other) const {
    return !(*this == other);
  }

private:
  friend void SerializeTo(const Plane& source, proto::terra::Plane& target);
  friend Plane ParseFrom(const proto::terra::Plane& world, serialize::To<Plane>);

  void InitNonpersistent();

private:
  PlaneId plane_id_;
  Surface surface_;
  // some regions are not really part of the surface. Like caves.
  std::vector<Cell> off_surface_;
  std::unordered_map<std::string, region::RegionPtr> region_index_;

  // TODO: Make atomic? Or just use uuid generator
  static inline int next_id_{0};
};


}

