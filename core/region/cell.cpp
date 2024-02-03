#include "cell.hpp"

namespace hs::region {

void SerializeTo(const Cell& source, proto::region::Cell& to) {
  to.Clear();
  to.set_terrain(source.GetTerrain());
}

Cell ParseFrom( const proto::region::Cell& from, serialize::To<Cell>) {
  Cell result;
  result.SetTerrain(from.terrain());

  return result;
}

}
