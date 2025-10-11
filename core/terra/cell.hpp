#pragma once

#include <terra/cell.pb.h>

#include <core/region/region.hpp>
#include <core/utils/serialize.hpp>
#include <memory>
#include <string>
#include <string_view>

namespace hs::terra {

template <typename BaseTypes>
class Plane;
template <typename BaseTypes>
class Cell;

template <typename BaseTypes>
void SerializeTo(const Cell<BaseTypes> &source,
                 proto::terra::Cell &proto_destination);
template <typename BaseTypes>
Cell<BaseTypes> ParseFrom(const proto::terra::Cell &source,
                          serialize::To<Cell<BaseTypes>>);

template <typename BaseTypes>
class Cell {
 public:
  using Region = region::Region<BaseTypes>;
  Cell() : region_(std::make_shared<Region>()) {}

  Cell(const Cell &) = delete;
  Cell &operator=(const Cell &) = delete;
  Cell(Cell &&other) : region_(std::move(other.region_)) {
    // don't leave other in invalid form
    other.region_ = std::make_shared<Region>();
  }
  Cell &operator=(Cell &&other) {
    if (this == &other) return *this;
    region_ = other.region_;
    other.region_ = std::make_shared<Region>();
    return *this;
  }

  auto &GetRegion() const { return *region_; }
  auto &GetRegion() { return *region_; }
  auto &GetRegionPtr() const { return region_; }
  const auto &GetRegionPtr() { return region_; }

  bool operator==(const Cell &other) const {
    return *region_ == *(other.region_);
  }

  bool operator!=(const Cell &other) const { return !(*this == other); }

 private:
  friend void SerializeTo<BaseTypes>(const Cell<BaseTypes> &source,
                                     proto::terra::Cell &proto_destination);
  friend Cell<BaseTypes> ParseFrom<BaseTypes>(const proto::terra::Cell &source,
                                              serialize::To<Cell<BaseTypes>>);
  friend Plane<BaseTypes>;

  // Call this method via World object, not directly
  auto SetRegion(Region region) { *region_ = std::move(region); }

 private:
  std::shared_ptr<Region> region_;
};

}  // namespace hs::terra

#include "cell.inl"
