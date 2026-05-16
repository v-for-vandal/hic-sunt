#pragma once

#include "cell.hpp"

#include <google/protobuf/util/message_differencer.h>

#include <core/utils/serialize.hpp>
#include <core/utils/serialize_containers.hpp>

namespace hs::region {

template <typename BaseTypes>
bool Cell<BaseTypes>::operator==(const Cell<BaseTypes> &other) const {
  if (!google::protobuf::util::MessageDifferencer::Equals(improvement_,
                                                          other.improvement_)) {
    return false;
  }

  return true;
}

template <typename BaseTypes>
void SerializeTo(const Cell<BaseTypes> &source, proto::region::Cell &to) {
  to.Clear();
  auto improvement_ptr = to.mutable_improvements()->Add();
  *improvement_ptr = source.GetImprovement();

}

template <typename BaseTypes>
Cell<BaseTypes> ParseFrom(const proto::region::Cell &from,
                          serialize::To<Cell<BaseTypes>>) {
  Cell<BaseTypes> result;
  if (from.improvements_size() > 0) {
    result.SetImprovement(from.improvements(0));
  }

  return result;
}

} // namespace hs::region
