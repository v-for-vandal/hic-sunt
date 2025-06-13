#pragma once

#include <absl/container/flat_hash_map.h>

#include <core/utils/string_token.hpp>
#include <core/types/std_base_types.hpp>

namespace hs::region {

// Profit and losses
template<typename BaseTypes = StdBaseTypes>
class PnlStatement {
public:
  using ResourceId = typename BaseTypes::StringId;

  // List of resources and their per-turn values. Positive means income,
  // negative means losses
  using ResourceFlow = absl::flat_hash_map<ResourceId, int>;

  const auto& GetProfit() const { return profit_; }
  const auto& GetLosses() const { return losses_; }

  auto& GetProfit() { return profit_; }
  auto& GetLosses() { return losses_; }

  ResourceFlow GetTotal() const;

private:
  // All values here are positive
  ResourceFlow profit_;
  // All values here are negative
  ResourceFlow losses_;
};

}

#include "pnl_statement.inl"
