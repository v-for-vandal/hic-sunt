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

 private:
  static std::expected<std::pair<std::string, std::vector<StringId>>, ErrorCode>
  PreprocessCode(const proto::ruleset::effect::Code& code);

  static void AppendDependencies(std::vector<StringId>& target,
                                 const std::vector<StringId>& source);

  ProtoEffect data_;
  StringId id_{};
  std::string effect_code_;
  std::string possible_code_;
  std::vector<StringId> dependencies_;
};

template <typename BaseTypes>
using EffectDefinitionPtr =
    utils::NonNullSharedPtr<EffectDefinition<BaseTypes>>;

template <typename BaseTypes>
using ConstEffectDefinitionPtr =
    utils::NonNullSharedPtr<const EffectDefinition<BaseTypes>>;

}  // namespace hs::ruleset

#include "effect.inl"
