#pragma once

#include <string>
#include <string_view>


#include <core/utils/serialize.hpp>

#include <fbs/world_generated.h>
#include <region/cell.pb.h>

namespace hs::region {
  /// One cell in region map
  class Cell {

  public:
    std::string_view GetTerrain() const { return terrain_type_; }
    void SetTerrain(std::string_view terrain) { terrain_type_ = terrain; }

    bool operator==(const Cell&) const = default;

  private:
    // TODO: optimize by sharing strings
    // or by replacing it with token
    std::string terrain_type_;
  };

void SerializeTo(const Cell& source, proto::region::Cell& to);
Cell ParseFrom( const proto::region::Cell& from, serialize::To<Cell>);

}

