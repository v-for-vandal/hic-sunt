#pragma once

#include <array>

#include <types/scope_type.pb.h>

#include <core/utils/enum_bitset.hpp>

namespace hs::types {

using ScopeType = proto::types::ScopeType;
using ScopeTypeSet = proto::types::ScopeTypeSet;
using ScopeTypeFilter = utils::EnumBitset<ScopeType, proto::types::ScopeType_MAX + 1>;
using ScopeTypeLinkTable = std::array<ScopeTypeFilter, proto::types::ScopeType_MAX + 1>;

constexpr ScopeTypeFilter ToScopeTypeFilter(ScopeType scope_type) {
  ScopeTypeFilter result;
  if (scope_type != ScopeType::SCOPE_TYPE_UNSPECIFIED) {
    result.set(scope_type);
  }
  return result;
}

constexpr ScopeTypeFilter ToScopeTypeFilter(ScopeTypeSet scope_type_set);

ScopeTypeFilter ToScopeTypeFilter(const proto::types::ScopeTypeFilter &scope_type_filter);

constexpr ScopeTypeLinkTable BuildScopeTypeLinkTable() {
  ScopeTypeLinkTable table{};
  /*
  enum ScopeType {
    SCOPE_TYPE_UNSPECIFIED = 0;
    SCOPE_TYPE_WORLD = 1;
    SCOPE_TYPE_PLANE = 2;
    SCOPE_TYPE_REGION = 4;
    SCOPE_TYPE_CELL = 5;
    SCOPE_TYPE_CIV = 6;
    SCOPE_TYPE_CITY = 7;
    SCOPE_TYPE_IMPROVEMENT = 8;
    SCOPE_TYPE_IMPROVEMENT_CLASS = 11;
    SCOPE_TYPE_ARMY = 9;
    SCOPE_TYPE_UNIT = 10;
    SCOPE_TYPE_UNIT_CLASS = 12;
    SCOPE_TYPE_JOB_CLASS = 13;
    SCOPE_TYPE_JOB = 14;
  }
  */
  table[ScopeType::SCOPE_TYPE_PLANE] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_WORLD);
  table[ScopeType::SCOPE_TYPE_REGION] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_PLANE);
  table[ScopeType::SCOPE_TYPE_CELL] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_REGION);
  table[ScopeType::SCOPE_TYPE_CIV] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_WORLD);
  table[ScopeType::SCOPE_TYPE_CITY] = ScopeTypeFilter::Make(
        ScopeType::SCOPE_TYPE_CIV
        // TODO: Decide relation between city and region
  );
  table[ScopeType::SCOPE_TYPE_IMPROVEMENT] = ScopeTypeFilter::Make(
      ScopeType::SCOPE_TYPE_CELL,
      ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS
  );
  table[ScopeType::SCOPE_TYPE_JOB] = ScopeTypeFilter::Make(
      ScopeType::SCOPE_TYPE_IMPROVEMENT,
      ScopeType::SCOPE_TYPE_JOB_CLASS
  );

  return table;
}

inline constexpr ScopeTypeLinkTable kScopeTypeLinkTable = BuildScopeTypeLinkTable();

constexpr const ScopeTypeFilter& AllowedTargets(ScopeType from) {
  return kScopeTypeLinkTable[static_cast<size_t>(from)];
}

constexpr bool CanLinkScopes(ScopeType from, ScopeType to) {
  return AllowedTargets(from).test(to);
}

}  // namespace hs::types

#include "scope_type.inl"
