#pragma once

#include "cell.hpp"

#include <google/protobuf/util/message_differencer.h>

#include <core/utils/serialize.hpp>
#include <core/utils/serialize_containers.hpp>

namespace hs::region {

template <typename BaseTypes>
bool Cell<BaseTypes>::operator==(const Cell<BaseTypes> &other) const {
  if (biome_ != other.biome_) {
    return false;
  }

  if (feature_ != other.feature_) {
    return false;
  }

  if (!google::protobuf::util::MessageDifferencer::Equals(improvement_,
                                                          other.improvement_)) {
    return false;
  }

  return true;
}

template <typename BaseTypes>
void SerializeTo(const Cell<BaseTypes> &source, proto::region::Cell &to) {
  to.Clear();
  to.set_biome(BaseTypes::ToProtoString(source.GetBiome()));
  to.add_features(BaseTypes::ToProtoString(source.GetFeature()));
  auto improvement_ptr = to.mutable_improvements()->Add();
  *improvement_ptr = source.GetImprovement();

  // convert dictionary to serializable type

  SerializeStringIdMapTo(BaseTypes{}, source.user_data_string_,
                         to.mutable_user_data_string());
  SerializeStringIdMapTo(BaseTypes{}, source.user_data_numeric_,
                         to.mutable_user_data_numeric());
}

template <typename BaseTypes>
Cell<BaseTypes> ParseFrom(const proto::region::Cell &from,
                          serialize::To<Cell<BaseTypes>>) {
  using StringId = typename BaseTypes::StringId;
  Cell<BaseTypes> result;
  result.SetBiome(ParseFrom(from.biome(), serialize::To<StringId>{}));
  if (from.features_size() > 0) {
    result.SetFeature(ParseFrom(from.features(0), serialize::To<StringId>{}));
  }
  if (from.improvements_size() > 0) {
    result.SetImprovement(from.improvements(0));
  }

  spdlog::info("parsing from protobuf, height is {}", from.height());

  // TODO: RM
  //result.SetHeight(from.height());

  ParseStringIdMapFrom(BaseTypes{}, from.user_data_string(),
                       result.user_data_string_);
  ParseStringIdMapFrom(BaseTypes{}, from.user_data_numeric(),
                       result.user_data_numeric_);

  return result;
}

} // namespace hs::region
