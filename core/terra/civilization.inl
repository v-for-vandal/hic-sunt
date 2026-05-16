#pragma once

#include "civilization.hpp"

namespace hs::terra {

template <typename BaseTypes>
Civilization<BaseTypes>::Civilization(StringId id) : Base(id) {}

template <typename BaseTypes>
std::expected<void, ErrorCode> Civilization<BaseTypes>::AddChildScope(const ScopePtr& scope) {
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
bool Civilization<BaseTypes>::HasChildScope(ScopeType scope_type, const StringId& id) const
{
    auto& scopes_of_type = child_scopes_[scope_type];
    auto fit = scopes_of_type.find(id);
    return fit != scopes_of_type.end();
}

template <typename BaseTypes>
auto Civilization<BaseTypes>::GetChildScope(ScopeType scope_type, const StringId& id) const -> ScopePtr
{
    auto st_fit = child_scopes_.find(scope_type);
    if (st_fit == child_scopes_.end()) {
        return {};
    }
    auto& scopes_of_type = st_fit->second;
    auto fit = scopes_of_type.find(id);
    if (fit != scopes_of_type.end()) {
        return fit->second;
    }

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
  return AddScope(scope);
}


template <typename BaseTypes>
std::expected<typename Civilization<BaseTypes>::ScopePtr, ErrorCode>
Civilization<BaseTypes>::GetOrCreateChildScope(ScopeType scope_type, const StringId& id) {
    // note: this function is not called often and at the moment, there is no need
    // for optimization
    if (HasScope(scope_type, id)) {
        return GetScope(scope_type, id);
    }

    return CreateScope(scope_type, id);
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
