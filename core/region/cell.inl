#include <google/protobuf/util/message_differencer.h>

#include <core/utils/serialize.hpp>

namespace hs::region {

template<typename BaseTypes>
bool Cell<BaseTypes>::operator==(const Cell<BaseTypes>& other) const {
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

template<typename BaseTypes>
double Cell<BaseTypes>::GetDataNumeric(StringId key) const noexcept {
    auto fit = user_data_numeric_.find(key);

    if (fit != user_data_numeric_.end()) {
        return 0.0;
    }

    return fit->second;
}

template<typename BaseTypes>
double Cell<BaseTypes>::SetDataNumeric(StringId key, double value) {
    auto& elem = user_data_numeric_[key];
    double res = elem;
    elem = value;

    return res;
}

template<typename BaseTypes>
bool Cell<BaseTypes>::HasDataNumeric(StringId key) const noexcept {
    return user_data_numeric_.contains(key);
}

template<typename BaseTypes>
auto Cell<BaseTypes>::GetDataString(StringId key) const noexcept -> const String& {
    auto fit = user_data_string_.find(key);

    if (fit != user_data_string_.end()) {
        static String empty_result;
        return empty_result;
    }

    return fit->second;

}

template<typename BaseTypes>
auto Cell<BaseTypes>::SetDataString(StringId key, String value) -> const String& {
    auto& elem = user_data_string_[key];
    elem = std::move(value);

    return elem;
}

template<typename BaseTypes>
bool Cell<BaseTypes>::HasDataString(StringId key) const noexcept {
    return user_data_string_.contains(key);
}


template<typename BaseTypes>
void SerializeTo(const Cell<BaseTypes>& source, proto::region::Cell& to) {
  to.Clear();
  to.set_biome(source.GetBiome());
  to.add_features(source.GetFeature());
  auto improvement_ptr = to.mutable_improvements()->Add();
  *improvement_ptr = source.GetImprovement();

  SerializeMapTo(source.user_data_string_, to.mutable_user_data_string());
  SerializeMapTo(source.user_data_numeric_, to.mutable_user_data_numeric());
}

template<typename BaseTypes>
Cell<BaseTypes> ParseFrom( const proto::region::Cell& from, serialize::To<Cell<BaseTypes>>) {
  Cell<BaseTypes> result;
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

