#pragma once

#include <string>
#include <memory>
#include <string_view>

#include <core/utils/serialize.hpp>
#include <core/region/region.hpp>

#include <terra/cell.pb.h>


namespace hs::terra {

  class Cell {
  public:
    Cell():
      region_(std::make_shared<region::Region>()) {}

    Cell(const Cell&) = delete;
    Cell& operator=(const Cell&) = delete;
    Cell(Cell&& other):
      region_(std::move(other.region_))
    {
      // don't leave other in invalid form
      other.region_ = std::make_shared<region::Region>();
    }
    Cell& operator=(Cell&& other) {
      region_ = other.region_;
      other.region_ = std::make_shared<region::Region>();
      return *this;
    }


    auto& GetRegion() const { return *region_; }
    auto& GetRegion() { return *region_; }
    auto& GetRegionPtr() const { return region_; }
    const auto& GetRegionPtr() { return region_; }
    auto SetRegion(region::Region region) {
      *region_ = std::move(region);
    }


  private:
    std::shared_ptr<region::Region> region_;

  };

void SerializeTo(const Cell& source, proto::terra::Cell& proto_destination);
Cell ParseFrom( const proto::terra::Cell& source,  serialize::To<Cell>);

}
