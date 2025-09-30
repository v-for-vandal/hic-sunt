#pragma once

#include <absl/container/flat_hash_map.h>
#include <scope/scope.pb.h>
#include <core/scope/variable_definition.hpp>

namespace hs::scope {


template <typename BaseTypes = StdBaseTypes>
class NumericVariable {
public:
  using StringId = typename BaseTypes::StringId;
  using NumericValue = typename BaseTypes::NumericValue;

  void AddModifiers(const StringId &key,
                       NumericValue add, NumericValue mult);

  /** \brief Get total addictive and multiplicative modifiers for this
   *  variable
   */
  void FillModifiers(NumericValue& add, NumericValue& mult) const {
    for(auto& [_,v] : add_modifiers_) {
      add += v;
    }

    for(auto& [_,v] : mult_modifiers_) {
      mult+= v;
    }
  }


private:
  absl::flat_hash_map<StringId, NumericValue> add_modifiers_;
  absl::flat_hash_map<StringId, NumericValue> mult_modifiers_;
};

template <typename BaseTypes = StdBaseTypes>
class StringVariable {
public:
  using String = typename BaseTypes::String;

private:
  String value_{};

};

template <typename BaseTypes> class Scope;

template <typename BaseTypes>
using ScopePtr = NonNullSharedPtr<Scope<BaseTypes>>;

/** \brief Scope is a collection of variables (numeric and strings)
 *
 * It allows working with variables as 'set of changes to'
 */
template <typename BaseTypes = StdBaseTypes> class Scope {
public:
  using StringId = typename BaseTypes::StringId;
  using String = typename BaseTypes::String;
  using NumericValue = typename BaseTypes::NumericValue;
  using VariableDefinitions = hs::scope::VariableDefinitions<BaseTypes>;
  using VariableDefinitionsPtr = hs::scope::VariableDefinitionsPtr<BaseTypes>;
  using ScopePtr = hs::scope::ScopePtr<BaseTypes>;

  /** \brief Create new scope with given id and given variable definitions
   *
   *  \param id is used to uniquely identify scope between serializations
   *         id can be empty if this is leaf scope, that can not be referenced
   *         by other scopes
   */
  Scope(StringId id, const VariableDefinitionsPtr& definitions);
  Scope(StringId id, const ScopePtr &parent);

  const ScopePtr& GetParent() const { return parent_; }
  void SetParent(const ScopePtr& parent) { parent_ = parent; }

  const VariableDefinitions *Definitions() const;

  double GetNumericValue(const StringId &variable);

  double GetStringValue(const StringId &variable);

  bool AddNumericModifier(const StringId &variable, const StringId &key,
                       NumericValue add, NumericValue mult);

private:
  void FillNumericModifiers(const StringId &variable,
    NumericValue& add, NumericValue& mult) const;

private:
  StringId id_;

  // We have only one parent and for every type of scope type of parent is
  // fixed. E.g. parent of region is always world
  // Benifit of having one parent scope set explicitly as parent is that it is
  // easier to write mods and extensions
  ScopePtr parent_;
  // All other 'parental' scopes are considered tag-scopes and are assigned
  // or removed as game progresses
  std::vector<ScopePtr> tag_scopes_;

  absl::flat_hash_map<StringId, NumericVariable> numeric_variables_;
  absl::flat_hash_map<StringId, StringVariable> string_variables_;

  VariableDefinitionsPtr definitions_{};

  // Note: it may be beneficial to replace it with prefix tree?

};

}

#include "scope.inl"
