#pragma once

#include <absl/container/flat_hash_map.h>

#include <core/session/session.hpp>
#include <core/types/std_base_types.hpp>

namespace hs::session {

template <typename BaseTypes = StdBaseTypes>
class EffectExecutionStatistics {
 public:
  using StringId = typename BaseTypes::StringId;

  struct Entry {
    size_t execution_count{0};
    size_t total_duration_ns{0};
    size_t total_changes{0};
    size_t failure_count{0};
    // Increased for every scope change that we failed to apply
    size_t partial_failure_count{0};

    Entry& operator+=(const Entry& other) {
      execution_count += other.execution_count;
      total_duration_ns += other.total_duration_ns;
      total_changes += other.total_changes;
      failure_count += other.failure_count;
      partial_failure_count += other.partial_failure_count;
      return *this;
    }
  };

  void RecordExecution(const StringId& effect_id, size_t duration_ns, size_t change_count);
  void RecordFailure(const StringId& effect_id);
  void RecordPartialFailure(const StringId& effect_id);
  void MergeFrom(const EffectExecutionStatistics& other);

  const auto& GetEntries() const noexcept { return entries_; }

 private:
  absl::flat_hash_map<StringId, Entry> entries_;
};

template <typename BaseTypes = StdBaseTypes>
class EffectExecutor {
 public:
  using ScopePtr = scope::ScopePtr<BaseTypes>;
  using EffectInstance = hs::session::EffectInstance<BaseTypes>;
  using Statistics = EffectExecutionStatistics<BaseTypes>;

  EffectExecutor() = default;

  template <typename WorldPtr, typename RuleSetPtr>
  Statistics Execute(Session<BaseTypes, WorldPtr, RuleSetPtr>& session, size_t current_time);
};

}  // namespace hs::session

#include "effect_executor.inl"
