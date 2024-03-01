#pragma once

#include <string>
#include <memory>
#include <string_view>

#include <core/utils/serialize.hpp>
#include <core/region/region.hpp>

#include <terra/cell.pb.h>


namespace hs::terra {

  class World;
  class Cell;

  void SerializeTo(const Cell& source, proto::terra::Cell& proto_destination);
  Cell ParseFrom( const proto::terra::Cell& source,  serialize::To<Cell>);

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
      if(this == &other) return *this;
      region_ = other.region_;
      other.region_ = std::make_shared<region::Region>();
      return *this;
    }


    auto& GetRegion() const { return *region_; }
    auto& GetRegion() { return *region_; }
    auto& GetRegionPtr() const { return region_; }
    const auto& GetRegionPtr() { return region_; }

  bool operator==(const Cell& other) const {
    return *region_ == *(other.region_);
  }

  bool operator!=(const Cell& other) const {
    return !(*this == other);
  }

  private:
    friend void SerializeTo(const Cell& source, proto::terra::Cell& proto_destination);
    friend Cell ParseFrom( const proto::terra::Cell& source,  serialize::To<Cell>);
    friend World;

    // Call this method via World object, not directly
    auto SetRegion(region::Region region) {
      *region_ = std::move(region);
    }


  private:
    std::shared_ptr<region::Region> region_;

  };


}
