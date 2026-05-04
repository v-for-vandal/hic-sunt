#pragma once

#include <vector>

#include <core/scope/scope.hpp>
#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include <expected>

namespace hs::scope {

template <typename BaseTypes = StdBaseTypes>
class ScopeChangeSet {
 public:
  using StringId = typename BaseTypes::StringId;
  using NumericValue = typename BaseTypes::NumericValue;
  using Scope = hs::scope::Scope<BaseTypes>;
  using ScopePtr = hs::scope::ScopePtr<BaseTypes>;

  explicit ScopeChangeSet(const ScopePtr& target_scope) : target_scope_(target_scope) {}

  std::expected<void, ErrorCode> SetNumericModifier(
      const StringId& variable, const StringId& key, NumericValue add,
      NumericValue mult);

  std::expected<void, ErrorCode> ChangeNumericModifier(
      const StringId& variable, const StringId& key, NumericValue add,
      NumericValue mult);

  std::expected<void, ErrorCode> SetStringModifier(
      const StringId& variable, const StringId& key, const StringId& value,
      NumericValue level);

  std::expected<void, ErrorCode> Apply(size_t modification_time) const;

  bool Empty() const noexcept { return operations_.empty(); }
  void Clear() { operations_.clear(); }

 private:
  enum class OperationKind {
    kSetNumericModifier,
    kChangeNumericModifier,
    kSetStringModifier,
  };

  struct Operation {
    OperationKind kind;
    StringId variable;
    StringId key;
    NumericValue add{0};
    NumericValue mult{0};
    StringId value{};
    NumericValue level{0};
  };

  std::expected<void, ErrorCode> ValidateNumericVariable(
      const StringId& variable, const StringId& key) const;
  std::expected<void, ErrorCode> ValidateStringVariable(
      const StringId& variable, const StringId& key) const;

  ScopePtr target_scope_;
  std::vector<Operation> operations_;
};

}  // namespace hs::scope

#include "scope_change_set.inl"
