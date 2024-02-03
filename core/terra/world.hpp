#pragma once

#include <core/geometry/surface.hpp>
#include <core/terra/cell.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/utils/serialize.hpp>

#include <flatbuffers/flatbuffers.h>
#include <fbs/world_generated.h>

namespace hs::terra {

class World;

::flatbuffers::Offset<fbs::World> SerializeTo(const World& source, ::flatbuffers::FlatBufferBuilder& fbb);
World ParseFrom(const fbs::World& world, serialize::To<World>);

class World {
public:
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
  using Surface = geometry::Surface<Cell, QRSCoordinateSystem>;
  using SurfaceView = geometry::SurfaceView<Cell, QRSCoordinateSystem>;

  World() = default;
  World(const World&) = delete;
  World(World&&) = default;
  World& operator=(const World&) = delete;
  World& operator=(World&&) = default;

  World(QRSCoordinateSystem::QDelta q_size, QRSCoordinateSystem::RDelta r_size);

  SurfaceView GetSurface() const { return surface_.view(); }
  SurfaceView GetSurface() { return surface_.view(); }

  void SetRuleSet(ruleset::RuleSet rules) {
    ruleset_ = std::move(rules);
  }

  const auto& GetRuleSet() const { return ruleset_; }

private:
  friend ::flatbuffers::Offset<fbs::World> SerializeTo(const World& source, ::flatbuffers::FlatBufferBuilder& fbb);
  friend World ParseFrom(const fbs::World& world, serialize::To<World>);

private:
  Surface surface_;
  ruleset::RuleSet ruleset_;
};


}
