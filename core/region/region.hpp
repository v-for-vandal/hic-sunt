#pragma once

#include <memory>

#include <absl/container/flat_hash_set.h>

#include <core/geometry/box.hpp>
#include <core/geometry/surface.hpp>
#include <core/region/cell.hpp>
#include <core/region/pnl_statement.hpp>
#include <core/region/types.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/comb.hpp>
#include <core/utils/enum_bitset.hpp>

#include <core/ruleset/ruleset.hpp>

#include <region/region.pb.h>

namespace hs::region {

template <typename BaseTypes>
void SerializeTo(const Region<BaseTypes> &source, proto::region::Region &to);

template <typename BaseTypes>
Region<BaseTypes> ParseFrom(const proto::region::Region &from,
                            serialize::To<Region<BaseTypes>>);

template <typename BaseTypes = StdBaseTypes> class Region {
public:
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSCoords = geometry::Coords<geometry::QRSCoordinateSystem>;
  using QRSBox = geometry::Box<geometry::QRSCoordinateSystem>;
  using QRSSize =
      typename geometry::Coords<geometry::QRSCoordinateSystem>::DeltaCoords;
  using Surface = geometry::Surface<Cell<BaseTypes>, QRSCoordinateSystem>;
  using SurfaceView =
      geometry::SurfaceView<Cell<BaseTypes>, QRSCoordinateSystem>;
  using StringId = typename BaseTypes::StringId;
  using String = typename BaseTypes::String;
  using PnlStatement = PnlStatement<BaseTypes>;

  Region();
  Region(const Region &) = delete;
  Region(Region &&) = default;
  Region &operator=(const Region &) = delete;
  Region &operator=(Region &&) = default;

  // Create hexagonal region. Regions are always centered around
  // (0,0,0) point and has given radius
  // TODO: Accept id in constructor instead of static next_id
  // Reason: next_id is not preserved when serializing
  explicit Region(const StringId &region_id, int radius);

  SurfaceView GetSurface() const { return surface_.view(); }
  SurfaceView GetSurface() { return surface_.view(); }
  const auto &GetSurfaceObject() const { return surface_; }

  const StringId &GetId() const { return id_; }
  // void SetId(const StringId& id) { id_ = id; }

  bool SetBiome(QRSCoords coords, const StringId &biome);
  std::vector<std::pair<StringId, int>> GetTopKBiomes(int k) const {
    return biome_count_.TopK(k);
  }

  bool SetHeight(QRSCoords coords, double height);
  std::pair<double, double> GetHeightRange() const noexcept {
    return height_minmax_.GetRange();
  }
  bool SetTemperature(QRSCoords coords, double temperature);
  std::pair<double, double> GetTemperatureRange() const noexcept {
    return temperature_minmax_.GetRange();
  }
  bool SetPrecipitation(QRSCoords coords, double precipitation);
  std::pair<double, double> GetPrecipitationRange() const noexcept {
    return precipitation_minmax_.GetRange();
  }

  bool SetFeature(QRSCoords coords, const StringId &biome);

  bool SetImprovement(QRSCoords coords, const StringId &improvement_type);

  bool SetCityId(const StringId &city_id);
  bool IsCity() const { return !BaseTypes::IsNullToken(city_id_); }
  const StringId &GetCityId() const { return city_id_; }

  // Returns container with coordinates of all improved cells
  const auto &GetImprovedCells() const { return cells_with_improvements_; }

  // TODO: Perhaphs this method should not be inside region?
  PnlStatement
  BuildPnlStatement(const ruleset::RuleSet<BaseTypes> &ruleset) const;

  // This is abstract storage for data. It never throws. If key is absent,
  // default value is returned.
  double GetDataNumeric(QRSCoords coords, const StringId &key) const noexcept;
  double SetDataNumeric(QRSCoords coords, const StringId &key, double value);
  bool HasDataNumeric(QRSCoords coords, const StringId &key) const noexcept;
  // Same, but for string. Strings are owned (and copied) internally
  const String &GetDataString(QRSCoords coords,
                              const StringId &key) const noexcept;
  const String &SetDataString(QRSCoords coords, const StringId &key,
                              const StringId &value);
  bool HasDataString(QRSCoords coords, const StringId &key) const noexcept;

  bool operator==(const Region &other) const;
  bool operator!=(const Region &other) const = default;

private:
  // === persistent data
  StringId id_;
  Surface surface_;
  StringId city_id_;
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
  utils::Comb<BaseTypes> biome_count_;
  std::unordered_map<StringId, size_t> feature_count_;
  absl::flat_hash_set<QRSCoords> cells_with_improvements_;
  utils::MinMax<double> height_minmax_;
  utils::MinMax<double> temperature_minmax_;
  utils::MinMax<double> precipitation_minmax_;

  static inline int next_id_{0};

  void BuildEphemeral();

private:
  friend void SerializeTo<BaseTypes>(const Region &source,
                                     proto::region::Region &to);
  friend Region ParseFrom<BaseTypes>(const proto::region::Region &from,
                                     serialize::To<Region>);
};

} // namespace hs::region

#include "region.inl"
