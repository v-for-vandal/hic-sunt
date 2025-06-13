#pragma once

#include <core/geometry/surface.hpp>
#include <core/region/region.hpp>
#include <core/region/types.hpp>
#include <core/terra/types.hpp>
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

// World is a collection of planes
class World {
public:
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
  using QRSSize = geometry::DeltaCoords<geometry::QRSCoordinateSystem>;
  using QRSBox = geometry::Box<geometry::QRSCoordinateSystem>;

  World() = default;
  World(const World&) = delete;
  World(World&&) = default;
  World& operator=(const World&) = delete;
  World& operator=(World&&) = default;

  PlanePtr GetPlane(PlaneIdCRef id) const;
  PlanePtr AddPlane(PlaneIdCRef id, QRSBox box);

  bool operator==(const World& other) const;
  bool operator!=(const World& other) const {
    return !(*this == other);
  }

private:
  friend void SerializeTo(const World& source, proto::terra::World& target);
  friend World ParseFrom(const proto::terra::World& world, serialize::To<World>);
  void InitNonpersistent();

private:
  std::unordered_map<std::string, PlanePtr> planes_;
};


}
