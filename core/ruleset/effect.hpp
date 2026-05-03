#pragma once

#include <ruleset/effect.pb.h>

#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
#include <core/types/scope_type.hpp>
#include <core/utils/non_null_ptr.hpp>
#include <expected>
#include <string>
#include <vector>

namespace hs::ruleset {

template <typename BaseTypes = StdBaseTypes>
class EffectDefinition {
 public:
  using StringId = typename BaseTypes::StringId;
  using ProtoEffect = proto::ruleset::effect::Effect;

  struct LuaVariable {
    std::string lua_name;
    StringId variable_id;
  };

  EffectDefinition() = default;
  explicit EffectDefinition(ProtoEffect data);

  const StringId& GetId() const noexcept { return id_; }
  types::ScopeType GetScopeType() const noexcept { return data_.scope_type(); }
  const std::string& GetEffectCode() const noexcept { return effect_code_; }
  const std::string& GetPossibleCode() const noexcept { return possible_code_; }
  const ProtoEffect& GetData() const noexcept { return data_; }
  const std::vector<StringId>& GetDependencies() const noexcept {
    return dependencies_;
  }
  const std::vector<LuaVariable>& GetLuaVariables() const noexcept {
    return lua_variables_;
  }

 private:
  struct PreprocessedCode {
    std::string code;
    std::vector<StringId> dependencies;
    std::vector<LuaVariable> lua_variables;
  };

  static std::expected<PreprocessedCode, ErrorCode> PreprocessCode(
      const proto::ruleset::effect::Code& code, size_t& next_var_index);

  static void AppendDependencies(std::vector<StringId>& target,
                                 const std::vector<StringId>& source);
  static void AppendLuaVariables(std::vector<LuaVariable>& target,
                                 const std::vector<LuaVariable>& source);

  ProtoEffect data_;
  StringId id_{};
  std::string effect_code_;
  std::string possible_code_;
  std::vector<StringId> dependencies_;
  std::vector<LuaVariable> lua_variables_;
};

template <typename BaseTypes>
using EffectDefinitionPtr =
    utils::NonNullSharedPtr<EffectDefinition<BaseTypes>>;

template <typename BaseTypes>
using ConstEffectDefinitionPtr =
    utils::NonNullSharedPtr<const EffectDefinition<BaseTypes>>;

}  // namespace hs::ruleset

#include "effect.inl"
