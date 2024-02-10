#pragma once

#include <core/geometry/surface.hpp>
#include <core/region/cell.hpp>
#include <core/utils/comb.hpp>

#include <region/region.pb.h>

namespace hs::region {

class Region;

void SerializeTo(const Region& source, proto::region::Region& to);
Region ParseFrom( const proto::region::Region& from, serialize::To<Region>);

class Region {
public:
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
  using QRSSize = typename geometry::Coords<geometry::QRSCoordinateSystem>::DeltaCoords;
  using Surface = geometry::Surface<Cell, QRSCoordinateSystem>;
  using SurfaceView = geometry::SurfaceView<Cell, QRSCoordinateSystem>;

  Region() = default;
  Region(const Region&) = delete;
  Region(Region&&) = default;
  Region& operator=(const Region&) = delete;
  Region& operator=(Region&&) = default;

  explicit Region(QRSSize size);

  SurfaceView GetSurface() const { return surface_.view(); }
  SurfaceView GetSurface() { return surface_.view(); }



  bool SetTerrain(QRSCoords coords, std::string_view terrain);
  std::vector<std::pair<std::string, int>> GetTopKTerrain(int k) const {
    return terrain_count_.TopK(k);
  }
  bool SetFeature(QRSCoords coords, std::string_view terrain);
  bool SetImprovement(QRSCoords coords, std::string_view terrain);

private:
  Surface surface_;
  utils::Comb terrain_count_;
  std::unordered_map<std::string, size_t> feature_count_;


private:
  friend void SerializeTo(const Region& source, proto::region::Region& to);
  friend Region ParseFrom( const proto::region::Region& from, serialize::To<Region>);
};


}
