#pragma once

#include <absl/container/flat_hash_set.h>

#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include <expected>
#include <vector>

namespace hs::ruleset {

template <typename BaseTypes = StdBaseTypes>
struct ParameterDomain {
  using StringId = typename BaseTypes::StringId;

  StringId name;
  std::vector<StringId> values;
};

template <typename BaseTypes = StdBaseTypes>
struct ParameterBinding {
  using StringId = typename BaseTypes::StringId;

  StringId name;
  StringId value;
};

template <typename BaseTypes = StdBaseTypes>
struct ParameterizedVariableInstance {
  using StringId = typename BaseTypes::StringId;

  StringId normalized_id;
  StringId variable_id;
  std::vector<ParameterBinding<BaseTypes>> parameters;
};

template <typename BaseTypes, typename ConcreteDefinition>
class ParameterizedVariableDefinition {
 public:
  using StringId = typename BaseTypes::StringId;

  struct Parameter {
    StringId name;
    std::vector<StringId> values;
    absl::flat_hash_set<StringId> allowed_values;
  };

  StringId id{};
  StringId pattern{};
  ConcreteDefinition concrete_definition{};
  std::vector<Parameter> parameters{};
};

inline constexpr size_t kMaxParameterizedVariableParameters = 5;

namespace details {

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>, ErrorCode>
ParseParameterizedDefinition(const typename BaseTypes::StringId& pattern,
                             std::vector<ParameterDomain<BaseTypes>> domains,
                             ConcreteDefinition definition);

template <typename BaseTypes>
std::expected<typename BaseTypes::StringId, ErrorCode> NormalizeConcreteOrQueryId(
    const typename BaseTypes::StringId& id, bool allow_wildcard);

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<void, ErrorCode> ValidateConcreteParameterBindings(
    const typename BaseTypes::StringId& id,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition);

template <typename BaseTypes, typename ConcreteDefinition>
typename BaseTypes::StringId BuildConcreteVariableId(
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition,
    const std::vector<typename BaseTypes::StringId>& parameter_values);

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<std::vector<ParameterizedVariableInstance<BaseTypes>>, ErrorCode>
GenerateParameterizedInstances(
    const typename BaseTypes::StringId& query,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition);

}  // namespace details

}  // namespace hs::ruleset

#include <core/ruleset/parameterized_variable_definition.inl>
