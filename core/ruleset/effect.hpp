#pragma once

#include <ruleset/effect.pb.h>

#include <core/types/std_base_types.hpp>
#include <core/utils/non_null_ptr.hpp>

namespace hs::ruleset {

template <typename BaseTypes = StdBaseTypes>
class EffectDefinition {
 public:
  using StringId = typename BaseTypes::StringId;
  using ProtoEffect = proto::ruleset::effect::Effect;
  using ProtoCode = proto::ruleset::effect::Code;

  EffectDefinition() = default;
  explicit EffectDefinition(ProtoEffect data)
      : data_(std::move(data)), id_(BaseTypes::StringIdFromStdString(data_.id())) {}

  const StringId& GetId() const noexcept;
  const ProtoCode& GetCode() const noexcept { return data_.code(); }
  const ProtoEffect& GetData() const noexcept { return data_; }

 private:
  ProtoEffect data_;
  StringId id_{};
};

template <typename BaseTypes>
using EffectDefinitionPtr =
    utils::NonNullSharedPtr<EffectDefinition<BaseTypes>>;

template <typename BaseTypes>
using ConstEffectDefinitionPtr =
    utils::NonNullSharedPtr<const EffectDefinition<BaseTypes>>;

}  // namespace hs::ruleset

#include "effect.inl"
