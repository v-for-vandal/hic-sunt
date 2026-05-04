#pragma once

#include <core/scope/scope.hpp>
#include <core/types/scope_type.hpp>
#include <core/types/std_base_types.hpp>

namespace hs::scope {

/** \brief Base (mixin) to add scope to object
 */
template <typename BaseTypes = StdBaseTypes>
class ScopedObject {
 public:
  using Scope = scope::Scope<BaseTypes>;
  using ScopePtr = scope::ScopePtr<BaseTypes>;
  using StringId = typename BaseTypes::StringId;

  ScopedObject() = default;
  ScopedObject(const StringId& id, types::ScopeType scope_type) : scope_(id, scope_type) {}

  const ScopePtr& GetScope() const { return scope_; }
  ScopePtr& GetScope() { return scope_; }

 protected:
  ScopePtr scope_;
};

// This is a helper class for cases when you known scope type in advance, at compile
// time. Those are rare, but it makes code easier to read.
template <typename BaseTypes, types::ScopeType scope_type>
struct TypedScopedObject : public ScopedObject<BaseTypes> {
    using StringId = typename BaseTypes::StringId;
    TypedScopedObject():
        TypedScopedObject(StringId{}) {}

    TypedScopedObject(StringId id):
        ScopedObject<BaseTypes>(id, scope_type) {}
};

}  // namespace hs::scope
