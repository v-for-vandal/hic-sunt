#pragma once

#include <string>
#include <string_view>

#include <absl/container/flat_hash_map.h>

#include <core/utils/serialize.hpp>
#include <core/types/std_base_types.hpp>

#include <fbs/world_generated.h>
#include <region/cell.pb.h>
#include <region/improvement.pb.h>

namespace hs::region {
  template<typename BaseTypes>
  class Region;
  template<typename BaseTypes>
  class Cell;

  template<typename BaseTypes>
  void SerializeTo(const Cell<BaseTypes>& source, proto::region::Cell& to);
  template<typename BaseTypes>
  Cell<BaseTypes> ParseFrom( const proto::region::Cell& from, serialize::To<Cell<BaseTypes>>);

  /// One cell in region map
  template<typename BaseTypes = StdBaseTypes>
  class Cell {

  public:
    using StringId = typename BaseTypes::StringId;
    using String = typename BaseTypes::String;

    StringId GetBiome() const { return biome_; }
    StringId GetFeature() const { return feature_; }

    // This is abstract storage for data. It never throws. If key is absent,
    // default value is returned.
    double GetDataNumeric(StringId key) const noexcept;
    double SetDataNumeric(StringId key, double value);
    bool HasDataNumeric(StringId key) const noexcept;
    // Same, but for string. Strings are owned (and copied) internally
    const String& GetDataString(StringId key) const noexcept;
    std::string SetDataString(StringId key, String value);
    bool HasDataString(StringId key) const noexcept;

    bool HasImprovement() const { return !improvement_.type().empty(); }
    const proto::region::Improvement& GetImprovement() const { return improvement_; }

    bool operator==(const Cell&) const;

  private:
    friend Region<BaseTypes>;
    friend void SerializeTo(const Cell<BaseTypes>& source, proto::region::Cell& to);
    friend Cell ParseFrom( const proto::region::Cell& from, serialize::To<Cell<BaseTypes>>);
    // Using this method is not recommended - instead use Region::SetBiome
    // because region tracks some aggregated information about cells
    void SetBiome(StringId biome) { biome_ = biome; }
    void SetFeature(StringId feature) { feature_ = feature; }
    void SetImprovement(proto::region::Improvement improvement) { improvement_ = improvement; }

  private:
    StringId biome_;
    StringId feature_;

    // height, in meters, above/under sea level (whatever sea this may be)
    // double height_meters_{0};
    // height, in abstract units
    // TODO: think and implement later

    proto::region::Improvement improvement_;

    absl::flat_hash_map<StringId, double> user_data_numeric_;
    absl::flat_hash_map<StringId, String> user_data_string_;
  };


}

#include "cell.inl"
