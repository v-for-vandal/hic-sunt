#pragma once

#include <absl/container/flat_hash_set.h>

#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include <expected>
#include <vector>

namespace hs::ruleset {

enum class ParameterDomainKind {
  kFixed,
  kOpen,
};

template <typename BaseTypes = StdBaseTypes>
struct ParameterDomain {
  using StringId = typename BaseTypes::StringId;

  StringId name;
  ParameterDomainKind kind{ParameterDomainKind::kFixed};
  std::vector<StringId> values;
};

template <typename BaseTypes = StdBaseTypes>
ParameterDomain<BaseTypes> FixedParameterDomain(typename BaseTypes::StringId name,
                                                std::vector<typename BaseTypes::StringId> values) {
  return ParameterDomain<BaseTypes>{
      .name = std::move(name),
      .kind = ParameterDomainKind::kFixed,
      .values = std::move(values),
  };
}

template <typename BaseTypes = StdBaseTypes>
ParameterDomain<BaseTypes> OpenParameterDomain(typename BaseTypes::StringId name) {
  return ParameterDomain<BaseTypes>{
      .name = std::move(name),
      .kind = ParameterDomainKind::kOpen,
      .values = {},
  };
}

template <typename BaseTypes = StdBaseTypes>
struct ParameterBinding {
  using StringId = typename BaseTypes::StringId;

  StringId name;
  StringId value;
};

enum class ParsedVariableQueryArgumentKind {
  kConcrete,
  kWildcard,
};

template <typename BaseTypes = StdBaseTypes>
struct ParsedVariableQueryArgument {
  using StringId = typename BaseTypes::StringId;

  ParsedVariableQueryArgumentKind kind{ParsedVariableQueryArgumentKind::kConcrete};
  StringId value{};
};

template <typename BaseTypes = StdBaseTypes>
struct ParsedVariableQuery {
  using StringId = typename BaseTypes::StringId;

  StringId raw_id{};
  StringId normalized_id{};
  bool is_parameterized{false};
  bool has_wildcard{false};
  std::vector<ParsedVariableQueryArgument<BaseTypes>> arguments{};
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
    ParameterDomainKind kind{ParameterDomainKind::kFixed};
    std::vector<StringId> values;
    absl::flat_hash_set<StringId> allowed_values;
  };

  StringId id{};
  StringId pattern{};
  ConcreteDefinition concrete_definition{};
  std::vector<Parameter> parameters{};
};

inline constexpr size_t kMaxParameterizedVariableParameters = 5;

template <typename BaseTypes>
std::expected<ParsedVariableQuery<BaseTypes>, ErrorCode> ParseVariableQuery(
    const typename BaseTypes::StringId& id, bool allow_wildcard = true);

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
    const ParsedVariableQuery<BaseTypes>& query,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition);

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<void, ErrorCode> ValidateConcreteParameterBindings(
    const typename BaseTypes::StringId& id,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition);

template <typename BaseTypes, typename ConcreteDefinition>
typename BaseTypes::StringId BuildConcreteVariableId(
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition,
    const std::vector<typename BaseTypes::StringId>& parameter_values);

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<typename BaseTypes::StringId, ErrorCode> BuildConcreteVariableId(
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition,
    const ParsedVariableQuery<BaseTypes>& query);

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<std::vector<ParameterizedVariableInstance<BaseTypes>>, ErrorCode>
GenerateParameterizedInstances(
    const typename BaseTypes::StringId& query,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition);

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<std::vector<ParameterizedVariableInstance<BaseTypes>>, ErrorCode>
GenerateParameterizedInstances(
    const ParsedVariableQuery<BaseTypes>& query,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition);

template <typename BaseTypes>
bool QueryMatchesConcreteId(const ParsedVariableQuery<BaseTypes>& query,
                            const ParsedVariableQuery<BaseTypes>& concrete_id);

}  // namespace details

}  // namespace hs::ruleset

#include <core/ruleset/parameterized_variable_definition.inl>
