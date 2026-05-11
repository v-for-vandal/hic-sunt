#pragma once

#include "effect.hpp"

#include <cctype>
#include <stdexcept>
#include <utility>

#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

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
int EffectDefinition<BaseTypes>::GetMaxOperations() const noexcept {
    // Later, we can add it to proto object. For now, constant will suffice
    return 10000;
}

template <typename BaseTypes>
EffectDefinition<BaseTypes>::EffectDefinition(ProtoEffect data)
    : data_(std::move(data)),
      id_(BaseTypes::StringIdFromStdString(data_.id())) {
  size_t next_var_index = 0;
  auto possible_processed = PreprocessCode(data_.possible(), next_var_index);
  if (!possible_processed) {
    throw std::system_error(make_error_code(possible_processed.error()));
  }

  auto effect_processed = PreprocessCode(data_.effect(), next_var_index);
  if (!effect_processed) {
    throw std::system_error(make_error_code(effect_processed.error()));
  }

  possible_code_ = WrapCodeInFunction(kPossibleFunctionName,
                                      possible_processed->code);
  effect_code_ = WrapCodeInFunction(kEffectFunctionName,
                                    effect_processed->code);
  AppendDependencies(dependencies_, possible_processed->dependencies);
  AppendDependencies(dependencies_, effect_processed->dependencies);
  AppendLuaVariables(lua_variables_, possible_processed->lua_variables);
  AppendLuaVariables(lua_variables_, effect_processed->lua_variables);

  auto possible_errors = ValidateLuaCode(id_, kPossibleFunctionName, possible_code_);
  auto effect_errors = ValidateLuaCode(id_, kEffectFunctionName, effect_code_);
  lua_errors_.reserve(possible_errors.size() + effect_errors.size());
  lua_errors_.insert(lua_errors_.end(),
                     std::make_move_iterator(possible_errors.begin()),
                     std::make_move_iterator(possible_errors.end()));
  lua_errors_.insert(lua_errors_.end(),
                     std::make_move_iterator(effect_errors.begin()),
                     std::make_move_iterator(effect_errors.end()));
  is_broken_ = !lua_errors_.empty();
}

template <typename BaseTypes>
std::string EffectDefinition<BaseTypes>::WrapCodeInFunction(
    std::string_view function_name, const std::string& code) {
  return "function " + std::string(function_name) + "(target)\n" + code +
         "\nend";
}

template <typename BaseTypes>
std::vector<std::string> EffectDefinition<BaseTypes>::ValidateLuaCode(
    const StringId& effect_id, std::string_view chunk_name,
    const std::string& wrapped_code) {
  std::vector<std::string> errors;

  if (wrapped_code.empty()) {
    return errors;
  }

  sol::state lua;
  lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table,
                     sol::lib::string);

  sol::load_result loaded = lua.load(wrapped_code);
  if (!loaded.valid()) {
    const auto error = loaded.get<sol::error>();
    auto message = fmt::format(
        "Failed to validate lua chunk {} for effect {}: {}", chunk_name,
        effect_id, error.what());
    spdlog::warn(message);
    errors.push_back(std::move(message));
    return errors;
  }

  sol::protected_function_result result = loaded();
  if (!result.valid()) {
    const auto error = result.get<sol::error>();
    auto message = fmt::format(
        "Failed to initialize lua chunk {} for effect {}: {}", chunk_name,
        effect_id, error.what());
    spdlog::warn(message);
    errors.push_back(std::move(message));
  }

  return errors;
}

template <typename BaseTypes>
void EffectDefinition<BaseTypes>::AppendDependencies(
    std::vector<StringId>& target, const std::vector<StringId>& source) {
  target.insert(target.end(), source.begin(), source.end());
}

template <typename BaseTypes>
void EffectDefinition<BaseTypes>::AppendLuaVariables(
    std::vector<LuaVariable>& target, const std::vector<LuaVariable>& source) {
  target.insert(target.end(), source.begin(), source.end());
}

template <typename BaseTypes>
auto EffectDefinition<BaseTypes>::PreprocessCode(
    const proto::ruleset::effect::Code& code, size_t& next_var_index)
    -> std::expected<PreprocessedCode, ErrorCode> {
  std::string source;
  if (code.has_lua()) {
    source = code.lua();
  }

  PreprocessedCode result;
  result.code.reserve(source.size());

  size_t pos = 0;
  while (pos < source.size()) {
    if (details::StartsWithAt(source, pos, "--[[")) {
      const size_t end = details::FindBlockCommentEnd(source, pos);
      result.code.append(source, pos, end - pos);
      pos = end;
      continue;
    }

    if (details::StartsWithAt(source, pos, "--")) {
      const size_t end = details::FindLineCommentEnd(source, pos);
      result.code.append(source, pos, end - pos);
      pos = end;
      continue;
    }

    if (source[pos] == '\'' || source[pos] == '"') {
      const size_t end = details::FindStringEnd(source, pos, source[pos]);
      result.code.append(source, pos, end - pos);
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

      const std::string lua_name = "__var_" + std::to_string(next_var_index++);
      result.code += lua_name;
      result.dependencies.push_back(BaseTypes::StringIdFromStdString(*parsed_argument));
      result.lua_variables.push_back(LuaVariable{
          .lua_name = lua_name,
          .variable_id = BaseTypes::StringIdFromStdString(*parsed_argument),
      });
      pos = argument_end + 1;
      continue;
    }

    result.code += source[pos];
    ++pos;
  }

  return result;
}

}  // namespace hs::ruleset
