#pragma once

#include <string>
#include <string_view>

#include <absl/container/flat_hash_map.h>
#include <spdlog/spdlog.h>

#include <core/types/std_base_types.hpp>
#include <core/scope/scope.hpp>
#include <core/scope/scoped_object.hpp>
#include <core/utils/minmax.hpp>
#include <core/utils/serialize.hpp>

#include <fbs/world_generated.h>
#include <region/cell.pb.h>
#include <region/improvement.pb.h>

namespace hs::region {
template <typename BaseTypes> class Region;
template <typename BaseTypes> class Cell;

template <typename BaseTypes>
void SerializeTo(const Cell<BaseTypes> &source, proto::region::Cell &to);
template <typename BaseTypes>
Cell<BaseTypes> ParseFrom(const proto::region::Cell &from,
                          serialize::To<Cell<BaseTypes>>);

/// One cell in region map
template <typename BaseTypes = StdBaseTypes> class Cell :
  public scope::ScopedObject<BaseTypes> {

public:
  using StringId = typename BaseTypes::StringId;
  using String = typename BaseTypes::String;


  StringId GetBiome() const { return biome_; }
  StringId GetFeature() const { return feature_; }
  /* TODO: Remove
   * it was replaced with variable in Scope
  */

  // This is abstract storage for data. It never throws. If key is absent,
  // default value is returned.
  /* TODO: REMOVE
  double GetDataNumeric(StringId key) const noexcept;
  double SetDataNumeric(StringId key, double value);
  bool HasDataNumeric(StringId key) const noexcept;
  // Same, but for string. Strings are owned (and copied) internally
  const String &GetDataString(StringId key) const noexcept;
  const String &SetDataString(StringId key, String value);
  bool HasDataString(StringId key) const noexcept;
  */

  bool HasImprovement() const { return !improvement_.type().empty(); }
  const proto::region::Improvement &GetImprovement() const {
    return improvement_;
  }

  /* TODO: REMOVE
   * replaced with vairable in Scope
  double GetHeight() const noexcept {
    return height_;
  }
  double GetTemperature() const noexcept { return temperature_; }
  double GetPrecipitation() const noexcept { return precipitation_; }
  */

  bool operator==(const Cell &) const;

private:
  friend Region<BaseTypes>;
  friend void SerializeTo<BaseTypes>(const Cell<BaseTypes> &source,
                                     proto::region::Cell &to);
  friend Cell ParseFrom<BaseTypes>(const proto::region::Cell &from,
                                   serialize::To<Cell<BaseTypes>>);
  // Using this method is not recommended - instead use Region::SetBiome
  // because region tracks some aggregated information about cells
  void SetBiome(StringId biome) { biome_ = biome; }
  void SetFeature(StringId feature) { feature_ = feature; }
  void SetImprovement(proto::region::Improvement improvement) {
    improvement_ = improvement;
  }
  /* TODO: REMOVE?
  void SetHeight(double value) noexcept {
    height_ = value;
  }
  /* TODO: REMOVE, replaced with modifiers
  void SetTemperature(double value) noexcept { temperature_ = value; }
  void SetPrecipitation(double value) noexcept { precipitation_ = value; }
  */

private:
  // Note: ScopePtr scope_ will be inherited from ScopedObject

  StringId biome_;
  StringId feature_;

  /* TODO: REMOVE
  // height, in meters, above/under sea level (whatever sea this may be)
  // negative is under sea level, positive is above sea level
  double height_{0};
  // temperature, in celsius
  double temperature_{0};
  // precipitation, in millimeters
  double precipitation_{0};
  */

  proto::region::Improvement improvement_;

  absl::flat_hash_map<StringId, double> user_data_numeric_;
  absl::flat_hash_map<StringId, String> user_data_string_;
};

} // namespace hs::region

#include "cell.inl"
