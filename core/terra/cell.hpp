#pragma once

#include <string>
#include <string_view>

#include <core/utils/serialize.hpp>
#include <core/region/region.hpp>

#include <terra/cell.pb.h>


namespace hs::terra {

  class Cell {
  public:
    auto& GetRegion() const { return region_; }
    auto& GetRegion() { return region_; }
    auto SetRegion(region::Region region) {
      region_ = std::move(region);
    }


  private:
    region::Region region_;

  };

void SerializeTo(const Cell& source, proto::terra::Cell& proto_destination);
Cell ParseFrom( const proto::terra::Cell& source,  serialize::To<Cell>);

}
