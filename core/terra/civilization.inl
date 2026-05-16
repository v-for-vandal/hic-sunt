#pragma once

#include "civilization.hpp"

namespace hs::terra {

template <typename BaseTypes>
Civilization<BaseTypes>::Civilization(StringId id) : Base(id) {}

template <typename BaseTypes>
std::expected<void, ErrorCode> Civilization<BaseTypes>::AddScope(const ScopePtr& scope) {
  if (!types::CanLinkScopes(scope->GetType(), this->GetScope()->GetType())) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_SCOPE_TYPE);
  }

  auto& scopes_of_type = child_scopes_[scope->GetType()];
  auto fit = scopes_of_type.find(scope->GetId());
  if (fit != scopes_of_type.end()) {
    if (fit->second != scope) {
      return std::unexpected(ErrorCode::ERR_SCOPE_ALREADY_EXISTS);
    }
    return {};
  }

  scopes_of_type.emplace(scope->GetId(), scope);
  return {};
}

template <typename BaseTypes>
std::expected<typename Civilization<BaseTypes>::ScopePtr, ErrorCode>
Civilization<BaseTypes>::FindOrCreate(ScopeType scope_type, const StringId& id) {
  if (!types::CanLinkScopes(scope_type, this->GetScope()->GetType())) {
    return std::unexpected(ErrorCode::ERR_INCORRECT_SCOPE_TYPE);
  }

  auto& scopes_of_type = child_scopes_[scope_type];
  auto fit = scopes_of_type.find(id);
  if (fit != scopes_of_type.end()) {
    return fit->second;
  }

  ScopePtr scope{id, scope_type};
  auto parent_result = scope->SetParent(this->GetScope());
  if (!parent_result) {
    return std::unexpected(parent_result.error());
  }

  auto [it, inserted] = scopes_of_type.emplace(id, scope);
  return it->second;
}

template <typename BaseTypes>
const typename Civilization<BaseTypes>::ScopedChildrenMap&
Civilization<BaseTypes>::GetChildScopes() const noexcept {
  return child_scopes_;
}

template <typename BaseTypes>
const typename Civilization<BaseTypes>::ScopeMap*
Civilization<BaseTypes>::FindScopesByType(ScopeType scope_type) const noexcept {
  auto fit = child_scopes_.find(scope_type);
  return fit == child_scopes_.end() ? nullptr : &fit->second;
}

}  // namespace hs::terra
