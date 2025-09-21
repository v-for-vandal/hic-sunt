#pragma once

#include <absl/container/flat_hash_map.h>
#include <types/scope.pb.h>

namespace hs::types {

class VariableDefinitions {};

template <typename BaseTypes = StdBaseTypes> class Variable {
  using NumericValue = typename BaseTypes::NumericValue;

  template <typename BaseTypes> class Scope;

  template <typename BaseTypes>
  using ScopePtr = std::shared_ptr<Scope<BaseTypes>>;

  /** \brief Scope is a collection of variables (numeric and strings)
   *
   * It allows working with variables as 'set of changes to'
   */
  template <typename BaseTypes = StdBaseTypes> class Scope {
  public:
    using StringId = typename BaseTypes::StringId;
    using String = typename BaseTypes::String;
    using NumericValue = typename BaseTypes::NumericValue;

    Scope(const VariableDefinitions *definitions);
    Scope(const ScopePtr &parent);

    const VariableDefinitions *Definitions() const;

    double GetNumericValue(const StringId &variable);
    double GetStringValue(const StringId &variable);
    bool AddNumericValue(const StringId &variable, const StringId &key,
                         double value);
    bool MultNumericValue(const StringId &variable, const StringId &key,
                          double value);

  private:
    const VariableDefinitions *definitions_{nullptr};

    absl::flat_hash_map<StringId, double> user_data_numeric_;

    // Note: it may be beneficial to replace it with prefix tree?

    void FillNumericComponents(const StringId &variable);
  };
}

#include "scope.inl"
