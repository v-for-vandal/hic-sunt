#pragma once

#include <types/scope_type.pb.h>

#include <core/utils/enum_bitset.hpp>

namespace hs::types {

using ScopeType = proto::types::ScopeType;
using ScopeTypeEnum = proto::types::ScopeType;
using ScopeTypeSet = proto::types::ScopeTypeSet;
using ScopeTypeFilter = utils::EnumBitset<ScopeTypeEnum, proto::types::ScopeType_MAX + 1>;

ScopeTypeFilter ToScopeTypeFilter(ScopeType scope_type);
ScopeTypeFilter ToScopeTypeFilter(ScopeTypeSet scope_type_set);
ScopeTypeFilter ToScopeTypeFilter(const proto::types::ScopeTypeFilter &scope_type_filter);

}  // namespace hs::types
