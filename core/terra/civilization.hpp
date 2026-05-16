#pragma once

#include <absl/container/flat_hash_map.h>

#include <core/scope/scope.hpp>
#include <core/scope/scoped_object.hpp>
#include <core/types/error_code.hpp>
#include <core/types/scope_type.hpp>
#include <core/types/std_base_types.hpp>
#include <expected>

namespace hs::terra {

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

  std::expected<void, ErrorCode> AddScope(const ScopePtr& scope);
  std::expected<ScopePtr, ErrorCode> FindOrCreate(ScopeType scope_type, const StringId& id);

  const ScopedChildrenMap& GetChildScopes() const noexcept;
  const ScopeMap* FindScopesByType(ScopeType scope_type) const noexcept;

 private:
  ScopedChildrenMap child_scopes_;
};

}  // namespace hs::terra

#include "civilization.inl"
