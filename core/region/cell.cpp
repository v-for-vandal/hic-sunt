#include "cell.hpp"

#include <google/protobuf/util/message_differencer.h>

namespace hs::region {

bool Cell::operator==(const Cell& other) const {
  if(terrain_type_ != other.terrain_type_) {
    return false;
  }

  if(feature_ != other.feature_) {
    return false;
  }

  if(!google::protobuf::util::MessageDifferencer::Equals(improvement_, other.improvement_)) {
    return false;
  }

  return true;
}


void SerializeTo(const Cell& source, proto::region::Cell& to) {
  to.Clear();
  to.set_terrain(source.GetTerrain());
  to.add_features(source.GetFeature());
  auto improvement_ptr = to.mutable_improvements()->Add();
  *improvement_ptr = source.GetImprovement();
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
