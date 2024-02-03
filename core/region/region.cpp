#include "region.hpp"

namespace hs::region {

Region::Region(QRSSize size):
  surface_(size)
  {
  }

void SerializeTo(const Region& source, proto::region::Region& target)
{
  target.Clear();
  SerializeTo(source.surface_, *target.mutable_surface());
}

Region ParseFrom(const proto::region::Region& region, serialize::To<Region>)
{
  Region result;
  if(region.has_surface()) {
    result.surface_ = ParseFrom(region.surface(), serialize::To<Region::Surface>{});
  }

  return result;
}
}
