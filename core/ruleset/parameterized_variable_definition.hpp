#pragma once

#include <absl/container/flat_hash_set.h>

#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include <expected>
#include <memory>
#include <vector>

namespace hs::ruleset {

enum class ParameterDomainKind {
  kFixed,
  kOpen,
};

enum class FixedParameterDomain {
  kJob,
  kResource,
};

template <typename BaseTypes = StdBaseTypes>
struct ParameterBinding {
  using StringId = typename BaseTypes::StringId;

  StringId name;
  StringId value;
};

template <typename BaseTypes = StdBaseTypes>
struct FixedParameterDomainValues {
  using StringId = typename BaseTypes::StringId;

  std::vector<StringId> ordered_values;
  absl::flat_hash_set<StringId> allowed_values;
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
class ParameterizedVariableDefinition : public ConcreteDefinition {
 public:
  using StringId = typename BaseTypes::StringId;

  struct Parameter {
    // Parameter name from the source pattern segment, e.g. "job" in
    // "job/{job}/consumes/{resource}". Empty for open "{}" segments.
    StringId name;
    // Fixed parameters refer to one of the built-in fixed domains below; open
    // parameters accept any valid concrete parameter id, e.g. "relation/{}".
    ParameterDomainKind kind{ParameterDomainKind::kFixed};
    // Built-in domain used when `kind == kFixed`, e.g. `kJob` for "{job}".
    FixedParameterDomain fixed_domain{FixedParameterDomain::kJob};
    // Shared fixed-domain values registered by VariableDefinitions via
    // SetFixedParameterDomainValues(). Reused by all parameters with the same
    // fixed domain: ordered_values is used for wildcard expansion, and
    // allowed_values is used for concrete id validation. Null for open
    // parameters and for fixed domains that were not registered yet.
    std::shared_ptr<const FixedParameterDomainValues<BaseTypes>> fixed_values;
  };

  // Original declaration pattern with named placeholders, e.g.
  // "job/{job}/consumes/{resource}". The inherited `id` is the normalized form.
  StringId pattern{};
  // Parameters in placeholder order. For the example above: job, then resource.
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
