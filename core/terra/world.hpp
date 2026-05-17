#pragma once

#include <fbs/world_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <terra/world.pb.h>

#include <core/geometry/surface.hpp>
#include <core/region/region.hpp>
#include <core/region/types.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/scope/scoped_object.hpp>
#include <core/terra/cell.hpp>
#include <core/terra/civilization.hpp>
#include <core/terra/plane.hpp>
#include <core/terra/types.hpp>
#include <core/types/control_object.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/serialize.hpp>

namespace hs::terra {

template <typename BaseTypes>
class World;

template <typename BaseTypes>
void SerializeTo(const World<BaseTypes> &source, proto::terra::World &target);
template <typename BaseTypes>
World<BaseTypes> ParseFrom(const proto::terra::World &world, serialize::To<World<BaseTypes>>);

// World is a central object, that contains all planes, players, civilizations
// and so on. Combined with ruleset, it constitutes game field.
// It is also root for the tree of scopes
template <typename BaseTypes = StdBaseTypes>
class World : public scope::TypedScopedObject<BaseTypes, types::ScopeType::SCOPE_TYPE_WORLD> {
 public:
  using Base = scope::TypedScopedObject<BaseTypes, types::ScopeType::SCOPE_TYPE_WORLD>;
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
  using QRSSize = geometry::DeltaCoords<geometry::QRSCoordinateSystem>;
  using QRSBox = geometry::Box<geometry::QRSCoordinateSystem>;
  using Plane = Plane<BaseTypes>;
  using PlanePtr = PlanePtr<BaseTypes>;
  using Civilization = Civilization<BaseTypes>;
  using CivilizationPtr = CivilizationPtr<BaseTypes>;
  using RegionPtr = region::RegionPtr<BaseTypes>;
  using StringId = BaseTypes::StringId;
  using String = BaseTypes::String;

  World() : Base("world.root") {}
  World(const World &) = delete;
  World(World &&) = default;
  World &operator=(const World &) = delete;
  World &operator=(World &&) = default;

  PlanePtr GetPlane(const StringId &id) const;
  PlanePtr AddPlane(const StringId &id, QRSBox box, int region_radius, int region_external_radius);
  auto &GetPlanes() noexcept { return planes_; }

  CivilizationPtr GetOrCreateCivilization(const StringId &id);
  bool HasCivilization(const StringId &id) const noexcept;
  /* \brief Method will return civilization object.
   *
   * Please note that CivilizationPtr is non-nullable. If such civilization doesn't exist, a new
   * detached scope will be returned.
   */
  CivilizationPtr GetCivilization(const StringId& id) const noexcept;
  auto &GetCivilizations() noexcept { return civilizations_; }
  const auto &GetCivilizations() const noexcept { return civilizations_; }

  RegionPtr GetRegionById(const StringId &region_id) const noexcept;
  bool HasRegion(const StringId &region_id) const noexcept;

  // Creates next integer, always unique
  auto GetNextId() const { return control_object_->GetNextId(); }

  bool operator==(const World &other) const;
  bool operator!=(const World &other) const { return !(*this == other); }

 private:
  friend void SerializeTo<BaseTypes>(const World &source, proto::terra::World &target);
  friend World ParseFrom<BaseTypes>(const proto::terra::World &world, serialize::To<World>);
  void InitNonpersistent();

 private:
  // ScopePtr scope_ is inherited from ScopedObject
  std::unordered_map<StringId, PlanePtr> planes_;
  std::unordered_map<StringId, CivilizationPtr> civilizations_;
  ControlObjectPtr control_object_;
};

}  // namespace hs::terra

#include "world.inl"
