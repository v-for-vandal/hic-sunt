#pragma once

#include <absl/container/flat_hash_map.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>

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
std::expected<typename BaseTypes::StringId, ErrorCode> NormalizeConcreteOrQueryView(
    View id, bool allow_wildcard) {
  using CharT = ViewCharT<View>;

  const auto segments = SplitPath(id);
  std::vector<NormalizedSegment<CharT>> normalized_segments;
  normalized_segments.reserve(segments.size());

  bool has_parameter = false;
  for (const auto segment : segments) {
    if (!IsParameterizedSegment(segment)) {
      normalized_segments.push_back(NormalizedSegment<CharT>{.text = segment});
      continue;
    }

    has_parameter = true;
    if (IsWildcardParameterizedSegment(segment)) {
      if (!allow_wildcard) {
        return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
      }
    } else if (!IsValidConcreteParameterValue(segment.substr(1))) {
      return std::unexpected(ErrorCode::ERR_INVALID_VARIABLE_REFERENCE);
    }
    normalized_segments.push_back(NormalizedSegment<CharT>{.is_placeholder = true});
  }

  if (!has_parameter) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  const auto normalized = JoinNormalizedPath(normalized_segments);
  return StringIdFromString<BaseTypes>(normalized);
}

template <typename BaseTypes>
std::expected<typename BaseTypes::StringId, ErrorCode> NormalizeConcreteOrQueryId(
    const typename BaseTypes::StringId& id, bool allow_wildcard) {
  const auto view_holder = MakeStringIdView<BaseTypes>(id);
  return NormalizeConcreteOrQueryView<BaseTypes>(GetView(view_holder), allow_wildcard);
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
    const typename BaseTypes::StringId& id,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition) {
  const auto id_view_holder = MakeStringIdView<BaseTypes>(id);
  const auto pattern_view_holder = MakeStringIdView<BaseTypes>(definition.pattern);
  const auto id_segments = SplitPath(GetView(id_view_holder));
  const auto pattern_segments = SplitPath(GetView(pattern_view_holder));
  if (id_segments.size() != pattern_segments.size()) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  size_t parameter_index = 0;
  for (size_t idx = 0; idx < pattern_segments.size(); ++idx) {
    const auto pattern_segment = pattern_segments[idx];
    const auto id_segment = id_segments[idx];

    if (!IsPlaceholderSegment(pattern_segment)) {
      if (pattern_segment != id_segment) {
        return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
      }
      continue;
    }

    if (parameter_index >= definition.parameters.size() || !IsParameterizedSegment(id_segment) ||
        IsWildcardParameterizedSegment(id_segment)) {
      return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
    }

    const auto parameter_value = StringIdFromView<BaseTypes>(id_segment.substr(1));
    const auto& parameter = definition.parameters[parameter_index];
    if (!parameter.allowed_values.contains(parameter_value)) {
      return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
    }
    ++parameter_index;
  }

  if (parameter_index != definition.parameters.size()) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  return {};
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
std::expected<std::vector<ParameterizedVariableInstance<BaseTypes>>, ErrorCode>
GenerateParameterizedInstances(
    const typename BaseTypes::StringId& query,
    const ParameterizedVariableDefinition<BaseTypes, ConcreteDefinition>& definition) {
  using StringId = typename BaseTypes::StringId;

  const auto query_view_holder = MakeStringIdView<BaseTypes>(query);
  const auto pattern_view_holder = MakeStringIdView<BaseTypes>(definition.pattern);
  const auto query_segments = SplitPath(GetView(query_view_holder));
  const auto pattern_segments = SplitPath(GetView(pattern_view_holder));
  if (query_segments.size() != pattern_segments.size()) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
  }

  std::vector<std::vector<StringId>> choices;
  choices.reserve(definition.parameters.size());
  size_t parameter_index = 0;
  for (size_t idx = 0; idx < pattern_segments.size(); ++idx) {
    const auto pattern_segment = pattern_segments[idx];
    const auto query_segment = query_segments[idx];

    if (!IsPlaceholderSegment(pattern_segment)) {
      if (pattern_segment != query_segment) {
        return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
      }
      continue;
    }

    if (parameter_index >= definition.parameters.size() || !IsParameterizedSegment(query_segment)) {
      return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
    }

    const auto& parameter = definition.parameters[parameter_index];
    if (IsWildcardParameterizedSegment(query_segment)) {
      choices.push_back(parameter.values);
    } else {
      const auto value = StringIdFromView<BaseTypes>(query_segment.substr(1));
      if (!parameter.allowed_values.contains(value)) {
        return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
      }
      choices.push_back({value});
    }
    ++parameter_index;
  }

  if (parameter_index != definition.parameters.size()) {
    return std::unexpected(ErrorCode::ERR_NO_SUCH_VARIABLE);
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

}  // namespace hs::ruleset::details
