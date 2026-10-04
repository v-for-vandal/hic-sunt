#pragma once

#include <fmt/format.h>
#include <types/scope_type.pb.h>

#include <array>
#include <core/utils/enum_bitset.hpp>

namespace hs::types {

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
    SCOPE_TYPE_IMPROVEMENT_GROUP = 15;
    SCOPE_TYPE_ARMY = 9;
    SCOPE_TYPE_UNIT = 10;
    SCOPE_TYPE_UNIT_CLASS = 12;
    SCOPE_TYPE_JOB_CLASS = 13;
    SCOPE_TYPE_JOB = 14;
    SCOPE_TYPE_JOB_GROUP = 16;
}
*/
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

ScopeTypeFilter ToScopeTypeFilter(const proto::types::ScopeTypeFilter& scope_type_filter);

constexpr ScopeTypeLinkTable BuildScopeTypeLinkTable() {
  ScopeTypeLinkTable table{};
  table[ScopeType::SCOPE_TYPE_PLANE] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_WORLD);
  table[ScopeType::SCOPE_TYPE_PLANE_CLASS] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_WORLD);
  table[ScopeType::SCOPE_TYPE_REGION] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_PLANE);
  table[ScopeType::SCOPE_TYPE_CELL] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_REGION);
  table[ScopeType::SCOPE_TYPE_CIV] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_WORLD);
  table[ScopeType::SCOPE_TYPE_CITY] =
      ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_CIV
                            // TODO: Decide relation between city and region
      );
  table[ScopeType::SCOPE_TYPE_IMPROVEMENT] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_CELL);
  table[ScopeType::SCOPE_TYPE_JOB] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_IMPROVEMENT);
  table[ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_CIV);
  table[ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_CIV);
  table[ScopeType::SCOPE_TYPE_JOB_CLASS] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_CIV);
  table[ScopeType::SCOPE_TYPE_JOB_GROUP] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_CIV);

  return table;
}

constexpr ScopeTypeLinkTable BuildScopeTypeTagLinkTable() {
  ScopeTypeLinkTable table{};
  table[ScopeType::SCOPE_TYPE_PLANE] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_PLANE_CLASS);
  table[ScopeType::SCOPE_TYPE_IMPROVEMENT] =
      ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS, ScopeType::SCOPE_TYPE_CITY);
  table[ScopeType::SCOPE_TYPE_IMPROVEMENT_CLASS] =
      ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP);
  table[ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP] =
      ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_IMPROVEMENT_GROUP);
  table[ScopeType::SCOPE_TYPE_JOB] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_JOB_CLASS);
  table[ScopeType::SCOPE_TYPE_JOB_CLASS] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_JOB_GROUP);
  table[ScopeType::SCOPE_TYPE_JOB_GROUP] = ScopeTypeFilter::Make(ScopeType::SCOPE_TYPE_JOB_GROUP);

  return table;
}

inline constexpr ScopeTypeLinkTable kScopeTypeLinkTable = BuildScopeTypeLinkTable();

constexpr const ScopeTypeFilter& AllowedTargets(ScopeType from) {
  return kScopeTypeLinkTable[static_cast<size_t>(from)];
}

constexpr bool CanLinkScopes(ScopeType from, ScopeType to) { return AllowedTargets(from).test(to); }

inline constexpr ScopeTypeLinkTable kScopeTypeTagLinkTable = BuildScopeTypeTagLinkTable();

constexpr const ScopeTypeFilter& AllowedTagTargets(ScopeType from) {
  return kScopeTypeTagLinkTable[static_cast<size_t>(from)];
}

constexpr bool CanTagLinkScopes(ScopeType from, ScopeType to) {
  return AllowedTagTargets(from).test(to);
}

}  // namespace hs::types

template <>
struct fmt::formatter<hs::types::ScopeType> : fmt::formatter<std::string_view> {
  auto format(hs::types::ScopeType value, format_context& ctx) const {
    return fmt::formatter<std::string_view>::format(hs::proto::types::ScopeType_Name(value), ctx);
  }
};

#include "scope_type.inl"
