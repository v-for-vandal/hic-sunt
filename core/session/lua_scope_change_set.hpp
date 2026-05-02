#pragma once

#include <sol/sol.hpp>

#include <core/scope/scope_change_set.hpp>
#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>

namespace hs::session {

template <typename BaseTypes = StdBaseTypes>
class LuaScopeChangeSet {
 public:
  using ScopeChangeSet = scope::ScopeChangeSet<BaseTypes>;
  using StringId = typename BaseTypes::StringId;
  using NumericValue = typename BaseTypes::NumericValue;

  LuaScopeChangeSet(ScopeChangeSet& target, StringId key)
      : target_(target), key_(std::move(key)) {}

  void SetNumericModifier(const StringId& variable, NumericValue add,
                          NumericValue mult);
  void ChangeNumericModifier(const StringId& variable, NumericValue add,
                             NumericValue mult);
  void SetStringModifier(const StringId& variable, const StringId& value,
                         NumericValue level);

 private:
  static sol::error MakeLuaError(ErrorCode error);

  ScopeChangeSet& target_;
  StringId key_;
};

}  // namespace hs::session

#include "lua_scope_change_set.inl"
