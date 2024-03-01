#pragma once

#include <memory>

#include <core/geometry/surface.hpp>
#include <core/region/cell.hpp>
#include <core/utils/comb.hpp>

#include <region/region.pb.h>

namespace hs::region {

class Region;

using RegionPtr = std::shared_ptr<Region>;

void SerializeTo(const Region& source, proto::region::Region& to);
Region ParseFrom( const proto::region::Region& from, serialize::To<Region>);

class Region {
public:
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
  using QRSSize = typename geometry::Coords<geometry::QRSCoordinateSystem>::DeltaCoords;
  using Surface = geometry::Surface<Cell, QRSCoordinateSystem>;
  using SurfaceView = geometry::SurfaceView<Cell, QRSCoordinateSystem>;

  Region();
  Region(const Region&) = delete;
  Region(Region&&) = default;
  Region& operator=(const Region&) = delete;
  Region& operator=(Region&&) = default;

  // Create hexagonal region
  explicit Region(int radius);

  SurfaceView GetSurface() const { return surface_.view(); }
  SurfaceView GetSurface() { return surface_.view(); }
  const auto& GetSurfaceObject() const { return surface_; }



  std::string_view GetId() const { return id_; }
  void SetId(std::string_view id) { id_ = id; }

  bool SetTerrain(QRSCoords coords, std::string_view terrain);
  std::vector<std::pair<std::string, int>> GetTopKTerrain(int k) const {
    return terrain_count_.TopK(k);
  }

  bool SetFeature(QRSCoords coords, std::string_view terrain);

  bool SetImprovement(QRSCoords coords, std::string_view terrain);

  bool SetCityId(std::string_view city_id);
  bool IsCity() const { return !city_id_.empty(); }
  std::string_view GetCityId() const { return city_id_; }

  bool operator==(const Region& other) const;
  bool operator!=(const Region& other) const = default;

private:
  std::string id_;
  Surface surface_;
  utils::Comb terrain_count_;
  std::unordered_map<std::string, size_t> feature_count_;
  std::string city_id_;

  static inline int next_id_{0};

  void InitNonpersistent();


private:
  friend void SerializeTo(const Region& source, proto::region::Region& to);
  friend Region ParseFrom( const proto::region::Region& from, serialize::To<Region>);
};


}
