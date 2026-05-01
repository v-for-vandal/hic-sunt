#pragma once

#include <ruleset/effect.pb.h>

#include <core/types/error_code.hpp>
#include <core/types/std_base_types.hpp>
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

  const StringId& GetId() const noexcept;
  const std::string& GetCode() const noexcept { return code_; }
  const ProtoEffect& GetData() const noexcept { return data_; }
  const std::vector<StringId>& GetDependencies() const noexcept {
    return dependencies_;
  }

 private:
  static std::expected<std::pair<std::string, std::vector<StringId>>, ErrorCode>
  PreprocessCode(const ProtoEffect& data);

  ProtoEffect data_;
  StringId id_{};
  std::string code_;
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
