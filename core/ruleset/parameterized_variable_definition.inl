#pragma once

#include <absl/container/flat_hash_map.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

#include "parameterized_variable_definition.hpp"

namespace hs::ruleset::details {

inline constexpr char kPathSeparator = '/';
inline constexpr char kParameterMarker = '@';
inline constexpr char kWildcard = '*';

template <typename CharT>
constexpr CharT AsChar(char value) {
  return static_cast<CharT>(value);
}

template <typename ViewHolder>
decltype(auto) GetView(const ViewHolder& holder) {
  if constexpr (requires { holder.view(); }) {
    return holder.view();
  } else {
    return holder;
  }
}

template <typename BaseTypes>
auto MakeStringIdView(const typename BaseTypes::StringId& value) {
  return BaseTypes::ToStringIdView(value);
}

template <typename View>
using ViewCharT = std::ranges::range_value_t<View>;

template <typename View>
std::vector<std::basic_string_view<ViewCharT<View>>> SplitPath(View path) {
  using CharT = ViewCharT<View>;

  std::vector<std::basic_string_view<CharT>> result;
  size_t consumed = 0;
  for (const auto segment : path | std::views::split(AsChar<CharT>(kPathSeparator))) {
    const size_t segment_size = static_cast<size_t>(std::ranges::distance(segment));
    result.emplace_back(path.data() + consumed, segment_size);
    consumed += segment_size + 1;
  }
  return result;
}

template <typename CharT>
bool EqualsAscii(std::basic_string_view<CharT> value, std::string_view ascii) {
  if (value.size() != ascii.size()) {
    return false;
  }

  return std::ranges::equal(
      value, ascii, {}, [](CharT ch) { return ch; }, [](char ch) { return AsChar<CharT>(ch); });
}

template <typename CharT>
bool ContainsAsciiChar(std::basic_string_view<CharT> value, char needle) {
  return std::ranges::find(value, AsChar<CharT>(needle)) != value.end();
}

template <typename CharT>
std::basic_string<CharT> JoinPath(const std::vector<std::basic_string_view<CharT>>& segments) {
  std::basic_string<CharT> result;
  if (segments.empty()) {
    return result;
  }

  size_t total_size = segments.size() - 1;
  std::ranges::for_each(
      segments | std::views::transform([](auto segment) { return segment.size(); }),
      [&total_size](size_t segment_size) { total_size += segment_size; });
  result.reserve(total_size);

  bool first = true;
  for (const auto segment : segments) {
    if (!first) {
      result.push_back(AsChar<CharT>(kPathSeparator));
    }
    first = false;
    result.append(segment);
  }

  return result;
}

template <typename BaseTypes, typename CharT>
typename BaseTypes::StringId StringIdFromView(std::basic_string_view<CharT> value) {
  return BaseTypes::StringIdFromStringView(value);
}

template <typename BaseTypes, typename CharT>
typename BaseTypes::StringId StringIdFromString(const std::basic_string<CharT>& value) {
  return StringIdFromView<BaseTypes>(std::basic_string_view<CharT>(value));
}

template <typename CharT>
bool IsPlaceholderSegment(std::basic_string_view<CharT> segment) {
  return segment.size() >= 3 && segment.front() == AsChar<CharT>('{') &&
         segment.back() == AsChar<CharT>('}');
}

template <typename CharT>
bool ContainsPlaceholderBrace(std::basic_string_view<CharT> segment) {
  return ContainsAsciiChar(segment, '{') || ContainsAsciiChar(segment, '}');
}

template <typename CharT>
bool IsParameterizedSegment(std::basic_string_view<CharT> segment) {
  return !segment.empty() && segment.front() == AsChar<CharT>(kParameterMarker);
}

template <typename CharT>
bool IsWildcardParameterizedSegment(std::basic_string_view<CharT> segment) {
  return segment.size() == 2 && segment[0] == AsChar<CharT>(kParameterMarker) &&
         segment[1] == AsChar<CharT>(kWildcard);
}

template <typename CharT>
bool IsValidConcreteParameterValue(std::basic_string_view<CharT> value) {
  return !value.empty() && !EqualsAscii(value, "*") && !ContainsAsciiChar(value, kPathSeparator);
}

template <typename CharT>
struct NormalizedSegment {
  bool is_placeholder{false};
  std::basic_string_view<CharT> text{};
};

template <typename CharT>
std::basic_string<CharT> JoinNormalizedPath(const std::vector<NormalizedSegment<CharT>>& segments) {
  const std::array<CharT, 2> placeholder{AsChar<CharT>('{'), AsChar<CharT>('}')};
  const std::basic_string_view<CharT> placeholder_view{placeholder.data(), placeholder.size()};

  std::vector<std::basic_string_view<CharT>> path_segments;
  path_segments.reserve(segments.size());
  std::ranges::transform(segments, std::back_inserter(path_segments),
                         [placeholder_view](const auto& segment) {
                           return segment.is_placeholder ? placeholder_view : segment.text;
                         });

  return JoinPath(path_segments);
}

template <typename BaseTypes, typename View>
std::expected<ParsedVariableQuery<BaseTypes>, ErrorCode> ParseVariableQueryView(
    const typename BaseTypes::StringId& raw_id, View id, bool allow_wildcard) {
  using CharT = ViewCharT<View>;
  const std::basic_string_view<CharT> id_view{id.data(), id.size()};

  ParsedVariableQuery<BaseTypes> result;
  result.raw_id = raw_id;

  if (!ContainsAsciiChar(id_view, kParameterMarker)) {
    result.normalized_id = raw_id;
    return result;
  }

  const auto segments = SplitPath(id_view);
  std::vector<NormalizedSegment<CharT>> normalized_segments;
  normalized_segments.reserve(segments.size());

  for (const auto segment : segments) {
    if (!IsParameterizedSegment(segment)) {
      normalized_segments.push_back(NormalizedSegment<CharT>{.text = segment});
      continue;
    }

    result.is_parameterized = true;
    normalized_segments.push_back(NormalizedSegment<CharT>{.is_placeholder = true});

    if (IsWildcardParameterizedSegment(segment)) {
      if (!allow_wildcard) {
        return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
      }
      result.has_wildcard = true;
      result.arguments.push_back(ParsedVariableQueryArgument<BaseTypes>{
          .kind = ParsedVariableQueryArgumentKind::kWildcard,
      });
      continue;
    }

    const auto value = segment.substr(1);
    if (!IsValidConcreteParameterValue(value)) {
      return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
    }
    result.arguments.push_back(ParsedVariableQueryArgument<BaseTypes>{
        .kind = ParsedVariableQueryArgumentKind::kConcrete,
        .value = StringIdFromView<BaseTypes>(value),
    });
  }

  if (result.is_parameterized) {
    const auto normalized = JoinNormalizedPath(normalized_segments);
    result.normalized_id = StringIdFromString<BaseTypes>(normalized);
  } else {
    result.normalized_id = raw_id;
  }

  return result;
}

template <typename BaseTypes, typename View>
std::expected<typename BaseTypes::StringId, ErrorCode> NormalizeConcreteOrQueryView(
    const typename BaseTypes::StringId& raw_id, View id, bool allow_wildcard) {
  auto parsed = ParseVariableQueryView<BaseTypes>(raw_id, id, allow_wildcard);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  if (!parsed->is_parameterized) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }
  return parsed->normalized_id;
}

template <typename BaseTypes>
std::expected<typename BaseTypes::StringId, ErrorCode> NormalizeConcreteOrQueryId(
    const typename BaseTypes::StringId& id, bool allow_wildcard) {
  const auto view_holder = MakeStringIdView<BaseTypes>(id);
  return NormalizeConcreteOrQueryView<BaseTypes>(id, GetView(view_holder), allow_wildcard);
}

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>, ErrorCode>
ParseParameterizedDefinition(const typename BaseTypes::StringId& pattern,
                             std::vector<ParameterDomain<BaseTypes>> domains,
                             ConcreteDefinition definition) {
  using StringId = typename BaseTypes::StringId;
  using ParameterizedDefinition = ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>;

  const auto pattern_view_holder = MakeStringIdView<BaseTypes>(pattern);
  const auto pattern_view = GetView(pattern_view_holder);
  using CharT = ViewCharT<decltype(pattern_view)>;
  const auto pattern_segments = SplitPath(pattern_view);

  std::vector<NormalizedSegment<CharT>> normalized_segments;
  normalized_segments.reserve(pattern_segments.size());
  std::vector<StringId> parameter_names;
  absl::flat_hash_set<StringId> seen_parameter_names;

  for (const auto segment : pattern_segments) {
    if (IsPlaceholderSegment(segment)) {
      const auto parameter_name =
          StringIdFromView<BaseTypes>(segment.substr(1, segment.size() - 2));
      if (BaseTypes::IsNullToken(parameter_name) ||
          !seen_parameter_names.insert(parameter_name).second) {
        return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);
      }
      parameter_names.push_back(parameter_name);
      normalized_segments.push_back(NormalizedSegment<CharT>{.is_placeholder = true});
      continue;
    }

    if (ContainsPlaceholderBrace(segment) || IsParameterizedSegment(segment)) {
      return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);
    }
    normalized_segments.push_back(NormalizedSegment<CharT>{.text = segment});
  }

  if (parameter_names.empty() || parameter_names.size() > kMaxParameterizedVariableParameters) {
    return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);
  }

  absl::flat_hash_map<StringId, ParameterDomain<BaseTypes>> domains_by_name;
  domains_by_name.reserve(domains.size());
  for (auto& domain : domains) {
    if (BaseTypes::IsNullToken(domain.name) ||
        !domains_by_name.try_emplace(domain.name, std::move(domain)).second) {
      return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);
    }
  }

  if (domains_by_name.size() != parameter_names.size()) {
    return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);
  }

  const auto normalized = JoinNormalizedPath(normalized_segments);
  const auto normalized_id = StringIdFromString<BaseTypes>(normalized);
  definition.id = normalized_id;

  ParameterizedDefinition result;
  result.id = normalized_id;
  result.pattern = pattern;
  result.concrete_definition = std::move(definition);
  result.parameters.reserve(parameter_names.size());

  for (const auto& parameter_name : parameter_names) {
    auto fit = domains_by_name.find(parameter_name);
    if (fit == domains_by_name.end()) {
      return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);
    }

    typename ParameterizedDefinition::Parameter parameter;
    parameter.name = parameter_name;
    parameter.kind = fit->second.kind;

    if (parameter.kind == ParameterDomainKind::kOpen) {
      if (!fit->second.values.empty()) {
        return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);
      }
      result.parameters.push_back(std::move(parameter));
      continue;
    }

    parameter.values.reserve(fit->second.values.size());
    parameter.allowed_values.reserve(fit->second.values.size());
    for (const auto& value : fit->second.values) {
      const auto value_view_holder = MakeStringIdView<BaseTypes>(value);
      if (!IsValidConcreteParameterValue(GetView(value_view_holder))) {
        return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_DEFINITION);
      }
      if (parameter.allowed_values.insert(value).second) {
        parameter.values.push_back(value);
      }
    }
    result.parameters.push_back(std::move(parameter));
  }

  return result;
}

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<void, ErrorCode> ValidateConcreteParameterBindings(
    const ParsedVariableQuery<BaseTypes>& query,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition) {
  if (!query.is_parameterized || query.has_wildcard || query.normalized_id != definition.id ||
      query.arguments.size() != definition.parameters.size()) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  for (size_t idx = 0; idx < definition.parameters.size(); ++idx) {
    const auto& argument = query.arguments[idx];
    if (argument.kind != ParsedVariableQueryArgumentKind::kConcrete) {
      return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
    }

    const auto& parameter = definition.parameters[idx];
    if (parameter.kind == ParameterDomainKind::kFixed &&
        !parameter.allowed_values.contains(argument.value)) {
      return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
    }
  }

  return {};
}

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<void, ErrorCode> ValidateConcreteParameterBindings(
    const typename BaseTypes::StringId& id,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition) {
  auto query = hs::ruleset::ParseVariableQuery<BaseTypes>(id, false);
  if (!query) {
    return std::unexpected(query.error());
  }
  return ValidateConcreteParameterBindings(*query, definition);
}

template <typename BaseTypes, typename ConcreteDefinition>
typename BaseTypes::StringId BuildConcreteVariableId(
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition,
    const std::vector<typename BaseTypes::StringId>& parameter_values) {
  const auto pattern_view_holder = MakeStringIdView<BaseTypes>(definition.pattern);
  const auto pattern_view = GetView(pattern_view_holder);
  using CharT = ViewCharT<decltype(pattern_view)>;
  const auto pattern_segments = SplitPath(pattern_view);
  std::basic_string<CharT> result;

  size_t parameter_index = 0;
  bool first = true;
  for (const auto pattern_segment : pattern_segments) {
    if (!first) {
      result.push_back(AsChar<CharT>(kPathSeparator));
    }
    first = false;

    if (!IsPlaceholderSegment(pattern_segment)) {
      result.append(pattern_segment);
      continue;
    }

    const auto value_view_holder = MakeStringIdView<BaseTypes>(parameter_values[parameter_index]);
    result.push_back(AsChar<CharT>(kParameterMarker));
    result.append(GetView(value_view_holder));
    ++parameter_index;
  }

  return StringIdFromString<BaseTypes>(result);
}

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<typename BaseTypes::StringId, ErrorCode> BuildConcreteVariableId(
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition,
    const ParsedVariableQuery<BaseTypes>& query) {
  auto validation = ValidateConcreteParameterBindings(query, definition);
  if (!validation) {
    return std::unexpected(validation.error());
  }

  std::vector<typename BaseTypes::StringId> values;
  values.reserve(query.arguments.size());
  for (const auto& argument : query.arguments) {
    values.push_back(argument.value);
  }
  return BuildConcreteVariableId(definition, values);
}

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<std::vector<ParameterizedVariableInstance<BaseTypes>>, ErrorCode>
GenerateParameterizedInstances(
    const ParsedVariableQuery<BaseTypes>& query,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition) {
  using StringId = typename BaseTypes::StringId;

  if (!query.is_parameterized || query.normalized_id != definition.id ||
      query.arguments.size() != definition.parameters.size()) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  std::vector<std::vector<StringId>> choices;
  choices.reserve(definition.parameters.size());
  for (size_t idx = 0; idx < definition.parameters.size(); ++idx) {
    const auto& argument = query.arguments[idx];
    const auto& parameter = definition.parameters[idx];
    if (argument.kind == ParsedVariableQueryArgumentKind::kWildcard) {
      if (parameter.kind == ParameterDomainKind::kOpen) {
        return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
      }
      choices.push_back(parameter.values);
      continue;
    }

    if (parameter.kind == ParameterDomainKind::kFixed &&
        !parameter.allowed_values.contains(argument.value)) {
      return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
    }
    choices.push_back({argument.value});
  }

  std::vector<ParameterizedVariableInstance<BaseTypes>> result;
  std::vector<StringId> selected_values(definition.parameters.size());

  const auto append_results = [&](const auto& self, size_t index) -> void {
    if (index == choices.size()) {
      ParameterizedVariableInstance<BaseTypes> instance;
      instance.normalized_id = definition.id;
      instance.variable_id = BuildConcreteVariableId(definition, selected_values);
      instance.parameters.reserve(definition.parameters.size());
      for (size_t parameter_idx = 0; parameter_idx < definition.parameters.size();
           ++parameter_idx) {
        instance.parameters.push_back(ParameterBinding<BaseTypes>{
            .name = definition.parameters[parameter_idx].name,
            .value = selected_values[parameter_idx],
        });
      }
      result.push_back(std::move(instance));
      return;
    }

    for (const auto& value : choices[index]) {
      selected_values[index] = value;
      self(self, index + 1);
    }
  };

  append_results(append_results, 0);
  return result;
}

template <typename BaseTypes, typename ConcreteDefinition>
std::expected<std::vector<ParameterizedVariableInstance<BaseTypes>>, ErrorCode>
GenerateParameterizedInstances(
    const typename BaseTypes::StringId& query,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition) {
  auto parsed = hs::ruleset::ParseVariableQuery<BaseTypes>(query, true);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }
  return GenerateParameterizedInstances(*parsed, definition);
}

template <typename BaseTypes>
bool QueryMatchesConcreteId(const ParsedVariableQuery<BaseTypes>& query,
                            const ParsedVariableQuery<BaseTypes>& concrete_id) {
  if (!query.is_parameterized || !concrete_id.is_parameterized || concrete_id.has_wildcard ||
      query.normalized_id != concrete_id.normalized_id ||
      query.arguments.size() != concrete_id.arguments.size()) {
    return false;
  }

  for (size_t idx = 0; idx < query.arguments.size(); ++idx) {
    const auto& query_argument = query.arguments[idx];
    if (query_argument.kind == ParsedVariableQueryArgumentKind::kWildcard) {
      continue;
    }
    if (concrete_id.arguments[idx].kind != ParsedVariableQueryArgumentKind::kConcrete ||
        concrete_id.arguments[idx].value != query_argument.value) {
      return false;
    }
  }

  return true;
}

}  // namespace hs::ruleset::details

namespace hs::ruleset {

template <typename BaseTypes>
std::expected<ParsedVariableQuery<BaseTypes>, ErrorCode> ParseVariableQuery(
    const typename BaseTypes::StringId& id, bool allow_wildcard) {
  const auto view_holder = details::MakeStringIdView<BaseTypes>(id);
  return details::ParseVariableQueryView<BaseTypes>(id, details::GetView(view_holder),
                                                    allow_wildcard);
}

}  // namespace hs::ruleset
