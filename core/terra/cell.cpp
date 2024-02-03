#include "cell.hpp"

namespace hs::terra {

void SerializeTo(const Cell& source, proto::terra::Cell& proto_destination)
{
  proto_destination.Clear();

  SerializeTo(source.GetRegion(), *proto_destination.mutable_region());

  return;
}

Cell ParseFrom( const proto::terra::Cell& source, serialize::To<Cell>) {
  Cell result;
  if(source.has_region()) {
    result.SetRegion(ParseFrom(source.region(), serialize::To<region::Region>{}));
  }

  return result;
}

}
