#pragma once

#include <core/scope/scope.hpp>
#include <core/types/std_base_types.hpp>

namespace hs::scope {


/** \brief Base (mixin) to add scope to object
 */
template <typename BaseTypes = StdBaseTypes>
class ScopedObject {
public:
  using Scope = scope::Scope<BaseTypes>;
  using ScopePtr = scope::ScopePtr<BaseTypes>;

  const ScopePtr& GetScope() const { return scope_; }
  ScopePtr& GetScope() { return scope_; }

protected:
  ScopePtr scope_;
};

}
