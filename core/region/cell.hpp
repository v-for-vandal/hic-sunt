#pragma once

#include <string>
#include <string_view>


#include <core/utils/serialize.hpp>

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
    std::string_view GetTerrain() const { return terrain_type_; }
    std::string_view GetFeature() const { return feature_; }

    bool HasImprovement() const { return !improvement_.type().empty(); }
    const proto::region::Improvement& GetImprovement() const { return improvement_; }

    bool operator==(const Cell&) const;

  private:
    friend Region;
    friend void SerializeTo(const Cell& source, proto::region::Cell& to);
    friend Cell ParseFrom( const proto::region::Cell& from, serialize::To<Cell>);
    // Using this method is not recommended - instead use Region::SetTerrain
    // because region tracks some aggregated information about cells
    void SetTerrain(std::string_view terrain) { terrain_type_ = terrain; }
    void SetFeature(std::string_view feature) { feature_ = feature; }
    void SetImprovement(proto::region::Improvement improvement) { improvement_ = improvement; }

  private:
    // TODO: optimize by sharing strings
    // or by replacing it with token
    std::string terrain_type_;
    std::string feature_;
    proto::region::Improvement improvement_;
  };


}

