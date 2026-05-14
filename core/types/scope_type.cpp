#include "scope_type.hpp"

namespace hs::types {

ScopeTypeFilter ToScopeTypeFilter(ScopeType scope_type) {
  ScopeTypeFilter result;
  if (scope_type != ScopeType::SCOPE_TYPE_UNSPECIFIED) {
    result.set(scope_type);
  }
  return result;
}

ScopeTypeFilter ToScopeTypeFilter(ScopeTypeSet scope_type_set) {
  ScopeTypeFilter result;
  switch (scope_type_set) {
    case ScopeTypeSet::SCOPE_TYPE_SET_GEO:
      result.set(ScopeType::SCOPE_TYPE_WORLD);
      result.set(ScopeType::SCOPE_TYPE_PLANE);
      result.set(ScopeType::SCOPE_TYPE_REGION);
      result.set(ScopeType::SCOPE_TYPE_CELL);
      break;
    case ScopeTypeSet::SCOPE_TYPE_SET_CIV:
      result.set(ScopeType::SCOPE_TYPE_PLAYER);
      result.set(ScopeType::SCOPE_TYPE_CITY);
      result.set(ScopeType::SCOPE_TYPE_IMPROVEMENT);
      result.set(ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS);
      break;
    case ScopeTypeSet::SCOPE_TYPE_SET_MILLITARY:
      result.set(ScopeType::SCOPE_TYPE_ARMY);
      result.set(ScopeType::SCOPE_TYPE_UNIT);
      result.set(ScopeType::SCOPE_TYPE_UNIT_CLASS);
      break;
    case ScopeTypeSet::SCOPE_TYPE_SET_UNSPECIFIED:
    default:
      break;
  }

  return result;
}

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
