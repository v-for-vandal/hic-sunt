#pragma once

#include <core/geometry/surface.hpp>
#include <core/region/region.hpp>
#include <core/region/types.hpp>
#include <core/terra/types.hpp>
#include <core/terra/cell.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/utils/serialize.hpp>
#include <core/types/std_base_types.hpp>

#include <flatbuffers/flatbuffers.h>
#include <fbs/world_generated.h>

#include <terra/world.pb.h>

namespace hs::terra {

template<typename BaseTypes>
class World;

template<typename BaseTypes>
void SerializeTo(const World<BaseTypes>& source, proto::terra::World& target);
template<typename BaseTypes>
World<BaseTypes> ParseFrom(const proto::terra::World& world, serialize::To<World<BaseTypes>>);

// World is a collection of planes
template<typename BaseTypes = StdBaseTypes>
class World {
public:
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
  using QRSSize = geometry::DeltaCoords<geometry::QRSCoordinateSystem>;
  using QRSBox = geometry::Box<geometry::QRSCoordinateSystem>;
  using Plane = Plane<BaseTypes>;
  using PlanePtr = PlanePtr<BaseTypes>;
  using StringId = BaseTypes::StringId;
  using String = BaseTypes::String;

  World() = default;
  World(const World&) = delete;
  World(World&&) = default;
  World& operator=(const World&) = delete;
  World& operator=(World&&) = default;

  PlanePtr GetPlane(const StringId& id) const;
  PlanePtr AddPlane(const StringId& id, QRSBox box);

  bool operator==(const World& other) const;
  bool operator!=(const World& other) const {
    return !(*this == other);
  }

private:
  friend void SerializeTo<BaseTypes>(const World& source, proto::terra::World& target);
  friend World ParseFrom<BaseTypes>(const proto::terra::World& world, serialize::To<World>);
  void InitNonpersistent();

private:
  std::unordered_map<StringId, PlanePtr> planes_;
};


}

#include "world.inl"
