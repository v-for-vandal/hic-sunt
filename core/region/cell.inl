#pragma once

#include <google/protobuf/util/message_differencer.h>

#include <core/utils/serialize.hpp>
#include <core/utils/serialize_containers.hpp>

#include "cell.hpp"

namespace hs::region {

template <typename BaseTypes>
void Cell<BaseTypes>::VisitScopes(this auto&& self, auto&& visitor) {
  visitor(self.GetScope());

  for (auto& [_, improvement] : self.improvements_) {
    visitor(improvement);
  }
}

template <typename BaseTypes>
bool Cell<BaseTypes>::operator==(const Cell<BaseTypes>& other) const {
  if (!google::protobuf::util::MessageDifferencer::Equals(improvement_, other.improvement_)) {
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
std::expected<void, ErrorCode> Cell<BaseTypes>::AddImprovement(int slot,
                                                               const ScopePtr& improvement) {
  if (!improvement->IsOrphaned()) {
    return std::unexpected(ErrorCode::ERR_IMPROVEMENT_SCOPE_ALREADY_HAS_PARENT);
  }

  if (HasImprovement(slot)) {
    return std::unexpected(ErrorCode::ERR_IMPROVEMENT_SLOT_OCCUPIED);
  }

  if (auto success = improvement->SetParent(this->GetScope()); !success) {
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
auto Cell<BaseTypes>::GetImprovement(int slot) const -> ScopePtr {
  const auto it = improvements_.find(slot);
  if (it == improvements_.end()) {
    return {};
  }

  return it->second;
}

template <typename BaseTypes>
void SerializeTo(const Cell<BaseTypes>& source, proto::region::Cell& to) {
  to.Clear();
  SerializeTo(*source.scope_, *to.mutable_scope());

  for (const auto& [slot, improvement] : source.improvements_) {
    if (improvement == nullptr) {
      continue;
    }
    auto* improvement_proto = to.add_improvements();
    improvement_proto->set_slot(slot);
    SerializeTo(*improvement, *improvement_proto->mutable_scope());
  }
}

template <typename BaseTypes>
Cell<BaseTypes> ParseFrom(const proto::region::Cell& from, serialize::To<Cell<BaseTypes>>) {
  Cell<BaseTypes> result;
  if (from.has_scope()) {
    result.scope_ = ParseFrom(from.scope(), serialize::To<typename Cell<BaseTypes>::Scope>{});
  }

  for (const auto& improvement_proto : from.improvements()) {
    auto improvement_scope = typename Cell<BaseTypes>::ScopePtr{
        ParseFrom(improvement_proto.scope(), serialize::To<typename Cell<BaseTypes>::Scope>{})};
    auto add_result = result.AddImprovement(improvement_proto.slot(), improvement_scope);
    if (!add_result) {
      spdlog::warn("Failed to restore improvement {} in slot {} for cell {}",
                   improvement_scope->GetId(), improvement_proto.slot(),
                   result.GetScope()->GetId());
    }
  }

  return result;
}

}  // namespace hs::region
