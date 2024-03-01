#pragma once

#include <core/geometry/surface.hpp>
#include <core/terra/cell.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/utils/serialize.hpp>

#include <flatbuffers/flatbuffers.h>
#include <fbs/world_generated.h>

#include <terra/world.pb.h>

namespace hs::terra {

class World;

void SerializeTo(const World& source, proto::terra::World& target);
World ParseFrom(const proto::terra::World& world, serialize::To<World>);

class World {
public:
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
  using QRSSize = geometry::DeltaCoords<geometry::QRSCoordinateSystem>;
  using Surface = geometry::Surface<Cell, QRSCoordinateSystem>;
  using SurfaceView = geometry::SurfaceView<Cell, QRSCoordinateSystem>;

  World() = default;
  World(const World&) = delete;
  World(World&&) = default;
  World& operator=(const World&) = delete;
  World& operator=(World&&) = default;

  World(
    QRSCoordinateSystem::QAxis q_start,
    QRSCoordinateSystem::QAxis q_end,
    QRSCoordinateSystem::RAxis r_start,
    QRSCoordinateSystem::RAxis r_end,
    QRSCoordinateSystem::SAxis s_start,
    QRSCoordinateSystem::SAxis s_end
    );
  //explicit World(QRSSize size);

  SurfaceView GetSurface() const { return surface_.view(); }
  SurfaceView GetSurface() { return surface_.view(); }

  const Surface& GetSurfaceObject() const { return surface_; }

  region::RegionPtr GetRegionById(const std::string& region_id) const;
  bool HasRegion(const std::string& region_id) const {
    return region_index_.contains(region_id);
  }
  void SetRegion(QRSCoords coords, region::Region region);

  bool operator==(const World& other) const;
  bool operator!=(const World& other) const {
    return !(*this == other);
  }

private:
  friend void SerializeTo(const World& source, proto::terra::World& target);
  friend World ParseFrom(const proto::terra::World& world, serialize::To<World>);

  void InitNonpersistent();

private:
  Surface surface_;
  // some regions are not really part of the surface. Like caves.
  // Or some magical land
  std::vector<Cell> off_surface_;
  //ruleset::RuleSet ruleset_;
  std::unordered_map<std::string, region::RegionPtr> region_index_;
};


}
