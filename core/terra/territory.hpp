#pragma once

#include <absl/container/flat_hash_set.h>

#include <core/region/types.hpp>

namespace hs::terra {

// World regions that are connected to each other
// Can work with off-surface regions as well
class ConnectedTerritory {
 public:
  // Return all regions bordering this one
  const absl::flat_hash_set<region::RegionId> &GetBorder();

  // Add region. Returns false if this new region is not connected
  // to territory
  bool AddRegion(region::RegionIdCRef new_region);

  // TODO: Remove region ?

 private:
  absl::flat_hash_set<hs::utils::StringToken> regions_;
};

// Set, possibly disjoin, of world region. There is no logic behind this
// grouping, other then game logic
class Territory {
 public:
 private:
  std::vector<ConnectedTerritory> connected_territories_;
};

}  // namespace hs::terra
