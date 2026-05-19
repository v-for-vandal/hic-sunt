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
bool Cell<BaseTypes>::HasImprovement(int slot) const {
  const auto it = improvements_.find(slot);
  if (it == improvements_.end()) {
    return false;
  }

  return !it->second->IsOrphaned();
}

template <typename BaseTypes>
std::expected<void, ErrorCode> Cell<BaseTypes>::AddImprovement(
    int slot, const ScopePtr& improvement) {
  if (!improvement->IsOrphaned()) {
    return std::unexpected(ErrorCode::ERR_IMPROVEMENT_SCOPE_ALREADY_HAS_PARENT);
  }

  if (HasImprovement(slot)) {
    return std::unexpected(ErrorCode::ERR_IMPROVEMENT_SLOT_OCCUPIED);
  }

  if(auto success = improvement->SetParent(this->GetScope()); !success) {
      return std::unexpected(success.error());
  }
  improvements_[slot] = improvement;
  return {};
}

template <typename BaseTypes>
typename Cell<BaseTypes>::StringId Cell<BaseTypes>::GetImprovementId(int slot) const {
  const auto it = improvements_.find(slot);
  if (it == improvements_.end() || it->second->IsOrphaned()) {
    return StringId{};
  }

  return it->second->GetId();
}

template <typename BaseTypes>
void SerializeTo(const Cell<BaseTypes> &source, proto::region::Cell &to) {
  to.Clear();
  /*
  auto improvement_ptr = to.mutable_improvements()->Add();
  *improvement_ptr = source.GetImprovement();
  */

}

template <typename BaseTypes>
Cell<BaseTypes> ParseFrom(const proto::region::Cell &from,
                          serialize::To<Cell<BaseTypes>>) {
  Cell<BaseTypes> result;
  /*
  if (from.improvements_size() > 0) {
    result.SetImprovement(from.improvements(0));
  }
  */

  return result;
}

} // namespace hs::region
