#pragma once

#include <absl/container/flat_hash_map.h>

#include <core/utils/string_token.hpp>

namespace hs::region {

using ResourceId = utils::StringToken;
using ResourceIdCRef = utils::StringTokenCRef;

// List of resources and their per-turn values. Positive means income,
// negative means losses
using ResourceFlow = absl::flat_hash_map<ResourceId, int>;

// Profit and losses
class PnlStatement {
public:
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
