#pragma once

#include <ruleset/effect.pb.h>

#include <core/ruleset/parameterized_variable_definition.hpp>
#include <core/types/error_code.hpp>
#include <core/types/scope_type.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/non_null_ptr.hpp>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hs::ruleset {

template <typename BaseTypes = StdBaseTypes>
class EffectDefinition {
 public:
  using StringId = typename BaseTypes::StringId;
  using ProtoEffect = proto::ruleset::effect::Effect;
  using ParsedVariableQuery = hs::ruleset::ParsedVariableQuery<BaseTypes>;

  struct LuaVariable {
    std::string lua_name;
    ParsedVariableQuery parsed_query;
  };

  struct Code {
    std::string code;
    std::vector<ParsedVariableQuery> dependencies;
    std::vector<LuaVariable> lua_variables;
  };

  EffectDefinition() = default;
  explicit EffectDefinition(ProtoEffect data);

  static constexpr std::string_view kPossibleFunctionName = "__hic_sunt_possible";
  static constexpr std::string_view kEffectFunctionName = "__hic_sunt_effect";

  const StringId& GetId() const noexcept { return id_; }
  types::ScopeType GetScopeType() const noexcept { return data_.scope_type(); }
  const Code& GetEffectCode() const noexcept { return effect_code_; }
  std::optional<Code> GePossibleCode() const noexcept { return possible_code_; }
  const ProtoEffect& GetData() const noexcept { return data_; }
  const std::vector<ParsedVariableQuery>& GetDependencies() const noexcept { return dependencies_; }
  const std::vector<LuaVariable>& GetLuaVariables() const noexcept { return lua_variables_; }

  bool IsBroken() const noexcept { return is_broken_; }
  const auto& GetLuaErrors() const noexcept { return lua_errors_; }

  int GetMaxOperations() const noexcept;

 private:
  static std::expected<Code, ErrorCode> PreprocessCode(const proto::ruleset::effect::Code& code,
                                                       size_t& next_var_index);
  static std::string WrapCodeInFunction(std::string_view function_name, const std::string& code);
  static std::vector<std::string> ValidateLuaCode(const StringId& effect_id,
                                                  std::string_view chunk_name,
                                                  const std::string& wrapped_code);

  static void AppendDependencies(std::vector<ParsedVariableQuery>& target,
                                 const std::vector<ParsedVariableQuery>& source);
  static void AppendLuaVariables(std::vector<LuaVariable>& target,
                                 const std::vector<LuaVariable>& source);

  ProtoEffect data_;
  StringId id_{};
  Code effect_code_;
  std::optional<Code> possible_code_;
  std::vector<ParsedVariableQuery> dependencies_;
  std::vector<LuaVariable> lua_variables_;
  bool is_broken_{false};
  std::vector<std::string> lua_errors_;
};

template <typename BaseTypes>
using EffectDefinitionPtr = utils::NonNullSharedPtr<EffectDefinition<BaseTypes>>;

template <typename BaseTypes>
using ConstEffectDefinitionPtr = utils::NonNullSharedPtr<const EffectDefinition<BaseTypes>>;

}  // namespace hs::ruleset

#include "effect.inl"
