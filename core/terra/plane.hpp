#pragma once

#include <terra/world.pb.h>

#include <core/geometry/surface.hpp>
#include <core/region/region.hpp>
#include <core/region/types.hpp>
#include <core/ruleset/ruleset.hpp>
#include <core/scope/scope.hpp>
#include <core/scope/scoped_object.hpp>
#include <core/terra/cell.hpp>
#include <core/terra/types.hpp>
#include <core/types/control_object.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/serialize.hpp>

namespace hs::terra {

template <typename BaseTypes>
class Plane;

template <typename BaseTypes>
class World;

template <typename BaseTypes>
void SerializeTo(const Plane<BaseTypes> &source, proto::terra::Plane &target);
template <typename BaseTypes>
Plane<BaseTypes> ParseFrom(const proto::terra::Plane &world,
                           serialize::To<Plane<BaseTypes>>);

// Plane is essentially one playable map that consists of multiple regions
template <typename BaseTypes = StdBaseTypes>
class Plane : public scope::ScopedObject<BaseTypes> {
 public:
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
  using QRSSize = geometry::DeltaCoords<geometry::QRSCoordinateSystem>;
  using QRSBox = geometry::Box<geometry::QRSCoordinateSystem>;
  using Region = region::Region<BaseTypes>;
  using RegionPtr = region::RegionPtr<BaseTypes>;
  using Cell = Cell<BaseTypes>;
  using Surface = geometry::Surface<Cell, QRSCoordinateSystem>;
  using SurfaceShape = geometry::SurfaceShape<QRSCoordinateSystem>;
  using SurfaceView = geometry::SurfaceView<Cell, QRSCoordinateSystem>;
  using StringId = BaseTypes::StringId;
  using String = BaseTypes::String;

  Plane() = default;
  Plane(const Plane &) = delete;
  Plane(Plane &&) = default;
  Plane &operator=(const Plane &) = delete;
  Plane &operator=(Plane &&) = default;

  Plane(ControlObjectPtr control_object, StringId plane_id, QRSBox box,
        // region_radius is the radius of playable region zone
        int region_radius,
        // external_region_radius is the radius of the region from the point of
        // view of global map. It is used primarily when calculating
        // 'real-world' distances for world generation, or some effect that
        // should degrade with distance to its source. The area is not rendered,
        // so you can put almost any value here, however large values will lead
        // to weird planes. Hexagon defined by external areas touch each other.
        // The distance between centers of two neighbouring hexagons is
        // 2*external radius If less then region_radius, or negative, we will
        // fallback to some reasonable value This parameter has no effect on
        // off-surface regions
        int external_region_radius = -1);

  const StringId &GetPlaneId() const { return plane_id_; }

  SurfaceView GetSurface() const { return surface_.view(); }
  SurfaceView GetSurface() { return surface_.view(); }

  const Surface &GetSurfaceObject() const { return surface_; }

  RegionPtr GetRegionById(const StringId &region_id) const;
  bool HasRegion(const StringId &region_id) const {
    return region_index_.contains(region_id);
  }
  void SetRegion(QRSCoords coords, Region region);
  RegionPtr GetRegion(QRSCoords coords) const;

  bool operator==(const Plane &other) const;
  bool operator!=(const Plane &other) const { return !(*this == other); }

  // == Plane-related functions

  // Distance between centers of two points in regions, expressed in number
  // of cells.
  // That is NOT movement distance and it shoud not be used to find movement-
  // related things, like 'closest city nearby' or 'how many turnes to get
  // to this mountain'. This value is used for world generation and related
  // areas. This function relies on external_region_radius for calculation.
  // TODO: This function should take into account plane wrapping
  // If some value is invalid (no such region or no such cell), then -1 is
  // returned.
  float GetDistanceBetweenCells(QRSCoords region1, QRSCoords cell1,
                                QRSCoords region2, QRSCoords cell2);

 private:
  friend void SerializeTo<BaseTypes>(const Plane &source,
                                     proto::terra::Plane &target);
  friend Plane ParseFrom<BaseTypes>(const proto::terra::Plane &world,
                                    serialize::To<Plane>);
  friend class World<BaseTypes>;

  void InitNonpersistent();

  void SetControlObject(const ControlObjectPtr &control_object) {
    control_object_ = control_object;
  }

 private:
  // ScopePtr scope_ will be inherited from ScopedObject
  ControlObjectPtr control_object_;
  StringId plane_id_;
  Surface surface_;
  int external_region_radius_{};
  // some regions are not really part of the surface. Like caves.
  std::vector<Cell> off_surface_;
  std::unordered_map<StringId, RegionPtr> region_index_;
};

}  // namespace hs::terra

#include "plane.inl"
