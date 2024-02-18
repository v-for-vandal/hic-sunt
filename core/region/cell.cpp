#include "cell.hpp"

namespace hs::region {

void SerializeTo(const Cell& source, proto::region::Cell& to) {
  to.Clear();
  to.set_terrain(source.GetTerrain());
  to.add_features(source.GetFeature());
  to.add_improvements(source.GetImprovement());
}

Cell ParseFrom( const proto::region::Cell& from, serialize::To<Cell>) {
  Cell result;
  result.SetTerrain(from.terrain());
  if(from.features_size() > 0) {
    result.SetFeature(from.features(0));
  }
  if(from.improvements_size() > 0) {
    result.SetImprovement(from.improvements(0));
  }

  return result;
}

}
