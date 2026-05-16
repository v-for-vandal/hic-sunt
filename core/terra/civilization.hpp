#pragma once

#include <absl/container/flat_hash_map.h>

#include <core/scope/scope.hpp>
#include <core/scope/scoped_object.hpp>
#include <core/types/error_code.hpp>
#include <core/types/scope_type.hpp>
#include <core/types/std_base_types.hpp>
#include <expected>

namespace hs::terra {

/* \brief Class that incapsulates working with scope of type SCOPE_TYPE_CIV
 *
 * This class is not intended to store all complexity of civilization - this logic lives in Godot.
 * Instead, we have here methods that simplify working with scopes.
 */
template <typename BaseTypes = StdBaseTypes>
class Civilization
    : public scope::TypedScopedObject<BaseTypes, types::ScopeType::SCOPE_TYPE_CIV> {
 public:
  using Base = scope::TypedScopedObject<BaseTypes, types::ScopeType::SCOPE_TYPE_CIV>;
  using Scope = scope::Scope<BaseTypes>;
  using ScopePtr = scope::ScopePtr<BaseTypes>;
  using ScopeType = types::ScopeType;
  using StringId = typename BaseTypes::StringId;
  using ScopeMap = absl::flat_hash_map<StringId, ScopePtr>;
  using ScopedChildrenMap = absl::flat_hash_map<ScopeType, ScopeMap>;

  Civilization() = default;
  explicit Civilization(StringId id);

  // Add given scope. Scope must be new, as it will be added as our child.
  std::expected<void, ErrorCode> AddChildScope(const ScopePtr& scope);
  // Creates new scope and adds it. If scope with this id exists (within this class), then
  // error is returned
  std::expected<ScopePtr, ErrorCode> CreateChildScope(ScopeType scope_type, const StringId& id);
  bool HasChildScope(ScopeType scope_type, const StringId& id) const;

  std::expected<ScopePtr, ErrorCode> GetOrCreateChildScope(ScopeType scope_type, const StringId& id);
  // This method will return default-initialized ScopePtr if no such id is present. Remember that
  // ScopePtr is non-nullable and will always contain data. It is very hard to check ScopePtr for
  // 'validness' - it is better to check HasScope before calling this method.
  ScopePtr GetChildScope(ScopeType scope_type, const StringId& id) const;

  const ScopedChildrenMap& GetChildScopes() const noexcept;
  const ScopeMap* FindScopesByType(ScopeType scope_type) const noexcept;

 private:
  ScopedChildrenMap child_scopes_;
};

}  // namespace hs::terra

#include "civilization.inl"
