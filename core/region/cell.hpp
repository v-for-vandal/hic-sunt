#pragma once

#include <string>
#include <string_view>

#include <absl/container/flat_hash_map.h>

#include <core/utils/serialize.hpp>
#include <core/utils/string_token.hpp>

#include <fbs/world_generated.h>
#include <region/cell.pb.h>
#include <region/improvement.pb.h>

namespace hs::region {
  class Region;
  class Cell;

  void SerializeTo(const Cell& source, proto::region::Cell& to);
  Cell ParseFrom( const proto::region::Cell& from, serialize::To<Cell>);

  /// One cell in region map
  class Cell {

  public:
    std::string_view GetBiome() const { return biome_; }
    std::string_view GetFeature() const { return feature_; }

    // This is abstract storage for data. It never throws. If key is absent,
    // default value is returned.
    double GetDataNumeric(std::string_view key) const noexcept;
    double SetDataNumeric(std::string_view key, double value);
    bool HasDataNumeric(std::string_view key) const noexcept;
    // Same, but for string. Strings are owned (and copied) internally
    std::string_view GetDataString(std::string_view key) const noexcept;
    std::string SetDataString(std::string_view key, std::string_view value);
    bool HasDataString(std::string_view key) const noexcept;

    bool HasImprovement() const { return !improvement_.type().empty(); }
    const proto::region::Improvement& GetImprovement() const { return improvement_; }

    bool operator==(const Cell&) const;

  private:
    friend Region;
    friend void SerializeTo(const Cell& source, proto::region::Cell& to);
    friend Cell ParseFrom( const proto::region::Cell& from, serialize::To<Cell>);
    // Using this method is not recommended - instead use Region::SetBiome
    // because region tracks some aggregated information about cells
    void SetBiome(std::string_view biome) { biome_ = biome; }
    void SetFeature(std::string_view feature) { feature_ = feature; }
    void SetImprovement(proto::region::Improvement improvement) { improvement_ = improvement; }

  private:
    // TODO: optimize by sharing strings
    // or by replacing it with token
    hs::utils::StringToken biome_;
    hs::utils::StringToken feature_;

    // height, in meters, above/under sea level (whatever sea this may be)
    // double height_meters_{0};
    // height, in abstract units
    // TODO: think and implement later

    proto::region::Improvement improvement_;

    absl::flat_hash_map<utils::StringToken, size_t> user_data_numeric_;
    absl::flat_hash_map<utils::StringToken, std::string> user_data_string_;
  };


}

