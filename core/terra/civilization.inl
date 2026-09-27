#pragma once

#include <core/utils/throw.hpp>

#include "civilization.hpp"

namespace hs::terra {

template <typename BaseTypes>
void Civilization<BaseTypes>::VisitScopes(this auto&& self, auto&& visitor) {
  visitor(self.GetScope());

  for (auto& [_, child_scope] : self.all_child_scopes_) {
    visitor(child_scope);
  }
}

template <typename BaseTypes>
Civilization<BaseTypes>::Civilization(StringId id) : Base(id) {}

template <typename BaseTypes>
std::expected<void, ErrorCode> Civilization<BaseTypes>::AddChildScope(const ScopePtr& scope) {
  if (!types::CanLinkScopes(scope->GetType(), this->GetScope()->GetType())) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_SCOPE_TYPE);
  }

  auto set_parent_success = scope->SetParent(this->GetScope());
  if (!set_parent_success) {
    return std::unexpected(set_parent_success.error());
  }
  {
    auto& scopes_of_type = child_scopes_[scope->GetType()];
    const auto& [fit, inserted] = scopes_of_type.try_emplace(scope->GetId(), scope);
    if (!inserted) {
      if (fit->second.get() != scope.get()) {
        return std::unexpected(ErrorCode::ERR_SCOPE_ALREADY_EXISTS);
      }
    }
  }

  {
    const auto& [fit, inserted] = all_child_scopes_.try_emplace(scope->GetId(), scope);
    if (!inserted) {
      if (fit->second.get() != scope.get()) {
        return std::unexpected(ErrorCode::ERR_SCOPE_ALREADY_EXISTS);
      }
    }
  }

  return {};
}

template <typename BaseTypes>
bool Civilization<BaseTypes>::HasChildScope(ScopeType scope_type, const StringId& id) const {
  auto st_fit = child_scopes_.find(scope_type);
  if (st_fit == child_scopes_.end()) {
    return false;
  }
  auto& scopes_of_type = st_fit->second;
  auto fit = scopes_of_type.find(id);
  return fit != scopes_of_type.end();
}

template <typename BaseTypes>
auto Civilization<BaseTypes>::GetChildScope(ScopeType scope_type, const StringId& id) const
    -> ScopePtr {
  auto st_fit = child_scopes_.find(scope_type);
  if (st_fit == child_scopes_.end()) {
    spdlog::warn(
        "Scope {} is not present. Try calling HasChildScope before GetChildScope or use "
        "GetOrCreateChildScope",
        id);
    return {};
  }
  auto& scopes_of_type = st_fit->second;
  auto fit = scopes_of_type.find(id);
  if (fit != scopes_of_type.end()) {
    return fit->second;
  }

  spdlog::warn(
      "Scope {} is not present. Try calling HasChildScope before GetChildScope or use "
      "GetOrCreateChildScope",
      id);
  return {};
}

template <typename BaseTypes>
std::expected<typename Civilization<BaseTypes>::ScopePtr, ErrorCode>
Civilization<BaseTypes>::CreateChildScope(ScopeType scope_type, const StringId& id) {
  if (!types::CanLinkScopes(scope_type, this->GetScope()->GetType())) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_SCOPE_TYPE);
  }

  auto& scopes_of_type = child_scopes_[scope_type];
  auto fit = scopes_of_type.find(id);
  if (fit != scopes_of_type.end()) {
    return std::unexpected(ErrorCode::ERR_SCOPE_ALREADY_EXISTS);
  }

  ScopePtr scope{id, scope_type};
  auto add_success = AddChildScope(scope);
  if (!add_success) {
    return std::unexpected(add_success.error());
  }

  return scope;
}

template <typename BaseTypes>
std::expected<typename Civilization<BaseTypes>::ScopePtr, ErrorCode>
Civilization<BaseTypes>::GetOrCreateChildScope(ScopeType scope_type, const StringId& id) {
  // note: this function is not called often and at the moment, there is no need
  // for optimization
  if (HasChildScope(scope_type, id)) {
    return GetChildScope(scope_type, id);
  }

  return CreateChildScope(scope_type, id);
}

template <typename BaseTypes>
auto Civilization<BaseTypes>::GetChildScopesByType() const noexcept -> const ScopedChildrenMap& {
  return child_scopes_;
}

template <typename BaseTypes>
auto Civilization<BaseTypes>::GetChildScopes() const noexcept -> const ScopeMap& {
  return all_child_scopes_;
}

template <typename BaseTypes>
const typename Civilization<BaseTypes>::ScopeMap* Civilization<BaseTypes>::FindScopesByType(
    ScopeType scope_type) const noexcept {
  auto fit = child_scopes_.find(scope_type);
  return fit == child_scopes_.end() ? nullptr : &fit->second;
}

template <typename BaseTypes>
void SerializeTo(const Civilization<BaseTypes>& source, proto::terra::Civilization& target) {
  target.Clear();
  SerializeTo(*source.scope_, *target.mutable_scope());
  for (const auto& [_, child_scope] : source.all_child_scopes_) {
    if (child_scope != nullptr) {
      SerializeTo(*child_scope, *target.add_child_scopes());
    }
  }
}

template <typename BaseTypes>
Civilization<BaseTypes> ParseFrom(const proto::terra::Civilization& source,
                                  serialize::To<Civilization<BaseTypes>>) {
  Civilization<BaseTypes> result;
  if (source.has_scope()) {
    result.scope_ =
        ParseFrom(source.scope(), serialize::To<typename Civilization<BaseTypes>::Scope>{});
  }

  for (const auto& child_scope_proto : source.child_scopes()) {
    typename Civilization<BaseTypes>::ScopePtr child_scope{
        ParseFrom(child_scope_proto, serialize::To<typename Civilization<BaseTypes>::Scope>{})};
    auto add_result = result.AddChildScope(child_scope);
    if (!add_result) {
      spdlog::warn("Failed to restore child scope {} for civilization {}", child_scope->GetId(),
                   result.GetId());
    }
  }

  return result;
}

}  // namespace hs::terra
