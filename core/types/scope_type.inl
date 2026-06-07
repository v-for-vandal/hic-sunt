#pragma once

#include "scope_type.hpp"

namespace hs::types {


    constexpr ScopeTypeFilter ToScopeTypeFilter(ScopeTypeSet scope_type_set) {
      ScopeTypeFilter result;
      switch (scope_type_set) {
        case ScopeTypeSet::SCOPE_TYPE_SET_GEO:
          result.set(ScopeType::SCOPE_TYPE_WORLD);
          result.set(ScopeType::SCOPE_TYPE_PLANE);
          result.set(ScopeType::SCOPE_TYPE_REGION);
          result.set(ScopeType::SCOPE_TYPE_CELL);
          break;
        case ScopeTypeSet::SCOPE_TYPE_SET_CIV:
          result.set(ScopeType::SCOPE_TYPE_CIV);
          result.set(ScopeType::SCOPE_TYPE_CITY);
          result.set(ScopeType::SCOPE_TYPE_IMPROVEMENT);
          result.set(ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS);
          break;
        case ScopeTypeSet::SCOPE_TYPE_SET_MILLITARY:
          result.set(ScopeType::SCOPE_TYPE_ARMY);
          result.set(ScopeType::SCOPE_TYPE_UNIT);
          result.set(ScopeType::SCOPE_TYPE_UNIT_CLASS);
          break;
        case ScopeTypeSet::SCOPE_TYPE_SET_JOBS:
          result |= ToScopeTypeFilter(ScopeTypeSet::SCOPE_TYPE_SET_GEO);
          result |= ToScopeTypeFilter(ScopeTypeSet::SCOPE_TYPE_SET_CIV);
          break;
        case ScopeTypeSet::SCOPE_TYPE_SET_UNSPECIFIED:
        default:
          break;
      }

      return result;
    }
}
