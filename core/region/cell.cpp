#include "cell.hpp"

#include <google/protobuf/util/message_differencer.h>

#include <core/utils/serialize.hpp>

namespace hs::region {

bool Cell::operator==(const Cell& other) const {
  if(biome_!= other.biome_) {
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

double Cell::GetDataNumeric(std::string_view key) const noexcept {
    auto fit = user_data_numeric_.find(key);

    if (fit != user_data_numeric_.end()) {
        return 0.0;
    }

    return fit->second;
}

double Cell::SetDataNumeric(std::string_view key, double value) {
    auto& elem = user_data_numeric_[key];
    double res = elem;
    elem = value;

    return res;
}

bool Cell::HasDataNumeric(std::string_view key) const noexcept {
    return user_data_numeric_.contains(key);
}

std::string_view Cell::GetDataString(std::string_view key) const noexcept {
    auto fit = user_data_string_.find(key);

    if (fit != user_data_string_.end()) {
        return {};
    }

    return fit->second;

}

std::string Cell::SetDataString(std::string_view key, std::string_view value) {
    auto& elem = user_data_string_[key];
    std::string res = elem;
    elem = value;

    return res;
}

bool Cell::HasDataString(std::string_view key) const noexcept {
    return user_data_string_.contains(key);
}


void SerializeTo(const Cell& source, proto::region::Cell& to) {
  to.Clear();
  to.set_biome(source.GetBiome());
  to.add_features(source.GetFeature());
  auto improvement_ptr = to.mutable_improvements()->Add();
  *improvement_ptr = source.GetImprovement();

  SerializeMapTo(source.user_data_string_, to.mutable_user_data_string());
  SerializeMapTo(source.user_data_numeric_, to.mutable_user_data_numeric());
}

Cell ParseFrom( const proto::region::Cell& from, serialize::To<Cell>) {
  Cell result;
  result.SetBiome(from.biome());
  if(from.features_size() > 0) {
    result.SetFeature(from.features(0));
  }
  if(from.improvements_size() > 0) {
    result.SetImprovement(from.improvements(0));
  }

  ParseMapFrom(from.user_data_string(), result.user_data_string_);
  ParseMapFrom(from.user_data_numeric(), result.user_data_numeric_);

  return result;
}

}
