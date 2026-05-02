#pragma once

#include "effect.hpp"

#include <cctype>
#include <stdexcept>
#include <utility>

namespace hs::ruleset {

namespace details {

inline bool IsValidEffectIdentifierChar(char ch) {
  const unsigned char uchar = static_cast<unsigned char>(ch);
  return std::isalnum(uchar) != 0 || ch == '_' || ch == '.';
}

inline bool IsValidEffectIdentifier(const std::string& token) {
  if (token.empty()) {
    return false;
  }

  for (char ch : token) {
    if (!IsValidEffectIdentifierChar(ch)) {
      return false;
    }
  }

  return true;
}

inline std::string TrimAsciiWhitespace(const std::string& input) {
  size_t begin = 0;
  while (begin < input.size() &&
         std::isspace(static_cast<unsigned char>(input[begin])) != 0) {
    ++begin;
  }

  size_t end = input.size();
  while (end > begin &&
         std::isspace(static_cast<unsigned char>(input[end - 1])) != 0) {
    --end;
  }

  return input.substr(begin, end - begin);
}

inline std::expected<std::string, ErrorCode> ParseVarArgument(
    const std::string& raw_argument) {
  const std::string trimmed = TrimAsciiWhitespace(raw_argument);
  if (trimmed.empty()) {
    return std::unexpected(ErrorCode::ERR_INVALID_EFFECT_VARIABLE_REFERENCE);
  }

  if (trimmed.front() == '"' || trimmed.back() == '"') {
    if (trimmed.size() < 2 || trimmed.front() != '"' ||
        trimmed.back() != '"') {
      return std::unexpected(ErrorCode::ERR_INVALID_EFFECT_VARIABLE_REFERENCE);
    }

    const std::string inner = trimmed.substr(1, trimmed.size() - 2);
    if (!IsValidEffectIdentifier(inner)) {
      return std::unexpected(ErrorCode::ERR_INVALID_EFFECT_VARIABLE_REFERENCE);
    }
    return inner;
  }

  if (!IsValidEffectIdentifier(trimmed)) {
    return std::unexpected(ErrorCode::ERR_INVALID_EFFECT_VARIABLE_REFERENCE);
  }

  return trimmed;
}

inline bool StartsWithAt(const std::string& source, size_t pos,
                         const char* needle) {
  for (size_t idx = 0; needle[idx] != '\0'; ++idx) {
    if (pos + idx >= source.size() || source[pos + idx] != needle[idx]) {
      return false;
    }
  }
  return true;
}

inline size_t FindStringEnd(const std::string& source, size_t pos, char quote) {
  ++pos;
  while (pos < source.size()) {
    if (source[pos] == '\\') {
      pos += 2;
      continue;
    }
    if (source[pos] == quote) {
      return pos + 1;
    }
    ++pos;
  }
  return source.size();
}

inline size_t FindLineCommentEnd(const std::string& source, size_t pos) {
  while (pos < source.size() && source[pos] != '\n') {
    ++pos;
  }
  return pos;
}

inline size_t FindBlockCommentEnd(const std::string& source, size_t pos) {
  pos += 4;  // skip --[[
  while (pos + 1 < source.size()) {
    if (source[pos] == ']' && source[pos + 1] == ']') {
      return pos + 2;
    }
    ++pos;
  }
  return source.size();
}

}  // namespace details

template <typename BaseTypes>
EffectDefinition<BaseTypes>::EffectDefinition(ProtoEffect data)
    : data_(std::move(data)),
      id_(BaseTypes::StringIdFromStdString(data_.id())) {
  auto possible_processed = PreprocessCode(data_.possible());
  if (!possible_processed) {
    throw std::system_error(make_error_code(possible_processed.error()));
  }

  auto effect_processed = PreprocessCode(data_.effect());
  if (!effect_processed) {
    throw std::system_error(make_error_code(effect_processed.error()));
  }

  possible_code_ = std::move(possible_processed->first);
  effect_code_ = std::move(effect_processed->first);
  AppendDependencies(dependencies_, possible_processed->second);
  AppendDependencies(dependencies_, effect_processed->second);
}

template <typename BaseTypes>
auto EffectDefinition<BaseTypes>::GetId() const noexcept -> const StringId& {
  return id_;
}

template <typename BaseTypes>
void EffectDefinition<BaseTypes>::AppendDependencies(
    std::vector<StringId>& target, const std::vector<StringId>& source) {
  target.insert(target.end(), source.begin(), source.end());
}

template <typename BaseTypes>
auto EffectDefinition<BaseTypes>::PreprocessCode(
    const proto::ruleset::effect::Code& code)
    -> std::expected<std::pair<std::string, std::vector<StringId>>, ErrorCode> {
  std::string source;
  if (code.has_lua()) {
    source = code.lua();
  }

  std::string result;
  std::vector<StringId> dependencies;
  result.reserve(source.size());

  size_t pos = 0;
  while (pos < source.size()) {
    if (details::StartsWithAt(source, pos, "--[[")) {
      const size_t end = details::FindBlockCommentEnd(source, pos);
      result.append(source, pos, end - pos);
      pos = end;
      continue;
    }

    if (details::StartsWithAt(source, pos, "--")) {
      const size_t end = details::FindLineCommentEnd(source, pos);
      result.append(source, pos, end - pos);
      pos = end;
      continue;
    }

    if (source[pos] == '\'' || source[pos] == '"') {
      const size_t end = details::FindStringEnd(source, pos, source[pos]);
      result.append(source, pos, end - pos);
      pos = end;
      continue;
    }

    if (details::StartsWithAt(source, pos, "VAR(")) {
      const size_t argument_begin = pos + 4;
      const size_t argument_end = source.find(')', argument_begin);
      if (argument_end == std::string::npos) {
        return std::unexpected(ErrorCode::ERR_INVALID_EFFECT_VARIABLE_REFERENCE);
      }

      auto parsed_argument = details::ParseVarArgument(
          source.substr(argument_begin, argument_end - argument_begin));
      if (!parsed_argument) {
        return std::unexpected(parsed_argument.error());
      }

      result += '"';
      result += *parsed_argument;
      result += '"';
      dependencies.push_back(BaseTypes::StringIdFromStdString(*parsed_argument));
      pos = argument_end + 1;
      continue;
    }

    result += source[pos];
    ++pos;
  }

  return std::make_pair(std::move(result), std::move(dependencies));
}

}  // namespace hs::ruleset
