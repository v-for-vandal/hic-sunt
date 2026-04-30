#pragma once

#include <absl/container/flat_hash_map.h>
#include <scope/scope.pb.h>

#include <core/scope/numeric_variable.hpp>
#include <core/scope/string_variable.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/non_null_ptr.hpp>
#include "core/ruleset/variable_definition.hpp"
// #include <core/scope/variable_definition.hpp>

namespace hs::scope {

template <typename BaseTypes>
class Scope;

template <typename BaseTypes>
void SerializeTo(const Scope<BaseTypes>& source, proto::scope::Scope& to);

template <typename BaseTypes>
Scope<BaseTypes> ParseFrom(const proto::scope::Scope& from,
                           serialize::To<Scope<BaseTypes>>);

template <typename BaseTypes>
using ScopePtr = utils::NonNullSharedPtr<Scope<BaseTypes>>;

/** \brief Scope is a collection of variables (numeric and strings)
 *
 * It allows working with variables as 'set of changes to'
 */
template <typename BaseTypes = StdBaseTypes>
class Scope {
 public:
  using StringId = typename BaseTypes::StringId;
  using String = typename BaseTypes::String;
  using NumericValue = typename BaseTypes::NumericValue;
  using NumericVariable = scope::NumericVariable<BaseTypes>;
  using StringVariable = scope::StringVariable<BaseTypes>;
  using ScopePtr = hs::scope::ScopePtr<BaseTypes>;
  using VariableDefinitionsPtr = hs::ruleset::VariableDefinitionsPtr<BaseTypes>;

  /** \brief Create new scope with given id and given variable definitions
   *
   *  \param id is used to uniquely identify scope between serializations
   *         id can be empty if this is leaf scope, that can not be referenced
   *         by other scopes
   */
  // Scope(StringId id, const VariableDefinitionsPtr& definitions);
  Scope(StringId id);
  Scope() : Scope(StringId{}) {}
  // Delete copying for now, we can do it, but we must propertly re-initialized
  // id because id must be unique
  Scope(const Scope&) = delete;
  Scope& operator=(const Scope&) = delete;
  Scope(Scope&&) = default;
  Scope& operator=(Scope&&) = default;

  const std::shared_ptr<Scope>& GetParent() const { return parent_; }
  void SetParent(const std::shared_ptr<Scope>& parent) { parent_ = parent; }

  // You can and should do it only on one root scope. All other scopes will fetch
  // it automatically
  void SetVariableDefinitions(const VariableDefinitionsPtr& definitions);

  // const VariableDefinitions *Definitions() const;

  std::expected<NumericValue, ErrorCode> GetNumericValue(const StringId& variable);

  std::expected<StringId, ErrorCode> GetStringValue(const StringId& variable);

  /*! \brief Sets modifier for given variable to given value(s)
   *
   * If modifier does not exist, it will be created
   */
  std::expected<void, ErrorCode> SetNumericModifier(const StringId& variable, const StringId& key,
                          NumericValue add, NumericValue mult,
                          size_t modificationTime = 0);

  /*! \brief Adjusts modifier for given variable by given difference.
   *
   * If modifier does not exist, it will be created
   */
  std::expected<void, ErrorCode> ChangeNumericModifier(const StringId& variable, const StringId& key,
                          NumericValue add, NumericValue mult,
                          size_t modificationTime = 0);

  std::expected<void, ErrorCode> SetStringModifier(const StringId& variable, const StringId& key,
                         const StringId& value, NumericValue level,
                         size_t modificationTime = 0);

  // Legacy compatibility wrappers
  bool AddNumericModifier(const StringId& variable, const StringId& key,
                          NumericValue add, NumericValue mult,
                          size_t modificationTime = 0) {
    return SetNumericModifier(variable, key, add, mult, modificationTime).has_value();
  }

  bool AddStringModifier(const StringId& variable, const StringId& key,
                         const StringId& value, NumericValue level,
                         size_t modificationTime = 0) {
    return SetStringModifier(variable, key, value, level, modificationTime).has_value();
  }

  // Returns abstract token that represents when this variable was last modified
  std::expected<size_t, ErrorCode> GetModificationTime(const StringId& variable) const;

  void ExplainNumericVariable(const StringId& variable, auto&& collect_fn);
  void ExplainStringVariable(const StringId& variable, auto&& collect_fn);
  void ExplainAllVariables(auto&& collect_fn);

  bool IsStringVariable(const StringId& variable) const;
  bool IsNumericVariable(const StringId& variable) const;

 private:
  void FillNumericModifiers(const StringId& variable, NumericValue& add,
                            NumericValue& mult) const;

  void FillStringModifiers(const StringId& variable, StringId& value,
                           NumericValue& level);

  const VariableDefinitionsPtr& GetVariableDefinitions() const;

 private:
  StringId id_;

  // We have only one parent and for every type of scope type of parent is
  // fixed. E.g. parent of region is always world
  // Benifit of having one parent scope set explicitly as parent is that it is
  // easier to write mods and extensions
  std::shared_ptr<Scope> parent_;
  // All other 'parental' scopes are considered tag-scopes and are assigned
  // or removed as game progresses
  std::vector<ScopePtr> tag_scopes_;

  absl::flat_hash_map<StringId, NumericVariable> numeric_variables_;
  absl::flat_hash_map<StringId, StringVariable> string_variables_;

  // Access it via special method that will pull it from parent
  mutable VariableDefinitionsPtr definitions_{};

  // Note: it may be beneficial to replace it with prefix tree?

 private:
  friend void SerializeTo<BaseTypes>(const Scope& source,
                                     proto::scope::Scope& to);
  friend Scope ParseFrom<BaseTypes>(const proto::scope::Scope& from,
                                    serialize::To<Scope>);
};

}  // namespace hs::scope

#include "scope.inl"
