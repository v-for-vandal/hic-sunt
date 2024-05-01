#pragma once

#include <memory>

#include <absl/container/flat_hash_set.h>

#include <core/geometry/surface.hpp>
#include <core/region/cell.hpp>
#include <core/region/types.hpp>
#include <core/region/pnl_statement.hpp>
#include <core/utils/comb.hpp>
#include <core/utils/enum_bitset.hpp>

#include <core/ruleset/ruleset.hpp>

#include <region/region.pb.h>

namespace hs::region {

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
  // TODO: Accept id in constructor instead of static next_id
  // Reason: next_id is not preserved when serializing
  explicit Region(int radius);

  SurfaceView GetSurface() const { return surface_.view(); }
  SurfaceView GetSurface() { return surface_.view(); }
  const auto& GetSurfaceObject() const { return surface_; }


  RegionIdCRef GetId() const { return id_; }
  void SetId(RegionIdCRef id) { id_ = id; }

  bool SetBiome(QRSCoords coords, std::string_view biome);
  std::vector<std::pair<std::string, int>> GetTopKBiomes(int k) const {
    return biome_count_.TopK(k);
  }

  bool SetFeature(QRSCoords coords, std::string_view biome);

  bool SetImprovement(QRSCoords coords, std::string_view improvement_type);

  bool SetCityId(std::string_view city_id);
  bool IsCity() const { return !city_id_.empty(); }
  std::string_view GetCityId() const { return city_id_; }

  // TODO: Perhaphs this method should not be inside region?
  PnlStatement BuildPnlStatement(const ruleset::RuleSet& ruleset) const;

  bool operator==(const Region& other) const;
  bool operator!=(const Region& other) const = default;

private:
  // === persistent data
  RegionId id_;
  Surface surface_;
  std::string city_id_;
  int next_unique_id_{0};

  enum class EphemeralData {
    kFeatureCount,
    kBiomeComb,
    kImprovements,

    kSize
  };

  // === ephemeral data

  utils::EnumBitset<EphemeralData> ephemeral_ready_;

  // can be updated if region was changed
  mutable PnlStatement current_pnl_;
  utils::Comb biome_count_;
  std::unordered_map<std::string, size_t> feature_count_;
  absl::flat_hash_set<QRSCoords> cells_with_improvements_;

  static inline int next_id_{0};

  void BuildEphemeral();


private:
  friend void SerializeTo(const Region& source, proto::region::Region& to);
  friend Region ParseFrom( const proto::region::Region& from, serialize::To<Region>);
};


}
