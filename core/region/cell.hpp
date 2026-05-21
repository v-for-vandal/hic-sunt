#pragma once

#include <absl/container/flat_hash_map.h>
#include <fbs/world_generated.h>
#include <region/cell.pb.h>
#include <region/improvement.pb.h>
#include <spdlog/spdlog.h>

#include <core/scope/scope.hpp>
#include <core/scope/scoped_object.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/minmax.hpp>
#include <core/utils/serialize.hpp>
#include <expected>
#include <string>
#include <string_view>

#include "core/types/scope_type.hpp"

namespace hs::region {
template <typename BaseTypes>
class Region;
template <typename BaseTypes>
class Cell;

template <typename BaseTypes>
void SerializeTo(const Cell<BaseTypes> &source, proto::region::Cell &to);
template <typename BaseTypes>
Cell<BaseTypes> ParseFrom(const proto::region::Cell &from, serialize::To<Cell<BaseTypes>>);

/// One cell in region map
template <typename BaseTypes = StdBaseTypes>
class Cell : public scope::TypedScopedObject<BaseTypes, types::ScopeType::SCOPE_TYPE_CELL> {
 public:
  using StringId = typename BaseTypes::StringId;
  using String = typename BaseTypes::String;
  using ScopePtr = scope::ScopePtr<BaseTypes>;

  Cell() = default;

  bool operator==(const Cell &) const;

  bool HasImprovement(int slot) const;
  std::expected<void, ErrorCode> AddImprovement(int slot, const ScopePtr& improvement);
  StringId GetImprovementId(int slot) const;

  void VisitScopes(this auto&& self, auto&& visitor);

 private:
  friend Region<BaseTypes>;
  friend void SerializeTo<BaseTypes>(const Cell<BaseTypes> &source, proto::region::Cell &to);
  friend Cell ParseFrom<BaseTypes>(const proto::region::Cell &from, serialize::To<Cell<BaseTypes>>);

 private:
     // map slot -> improvement in this slot
     absl::flat_hash_map<int, ScopePtr> improvements_;

     // TODO: RM
  proto::region::Improvement improvement_;
};

}  // namespace hs::region

#include "cell.inl"
