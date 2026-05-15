#include "scope_type.hpp"

namespace hs::types {



ScopeTypeFilter ToScopeTypeFilter(const proto::types::ScopeTypeFilter &scope_type_filter) {
  ScopeTypeFilter result;

  for (const int scope_type_set : scope_type_filter.scope_type_sets()) {
    if (proto::types::ScopeTypeSet_IsValid(scope_type_set)) {
      result |= ToScopeTypeFilter(static_cast<ScopeTypeSet>(scope_type_set));
    }
  }

  for (const int scope_type : scope_type_filter.scope_types()) {
    if (proto::types::ScopeType_IsValid(scope_type)) {
      result |= ToScopeTypeFilter(static_cast<ScopeType>(scope_type));
    }
  }

  return result;
}

}  // namespace hs::types
