#pragma once

#include <memory>
#include <filesystem>

#include <core/ruleset/ruleset.hpp>
#include <core/terra/world.hpp>

namespace hs::system {

struct NewWorldParameters {
  using QRSCoordinateSystem = geometry::QRSCoordinateSystem;
  using QRSSize = geometry::DeltaCoords<QRSCoordinateSystem>;

  QRSSize world_size;
  int region_size{1};
};

/* Central component for game */
class System {
public:
  using ErrorsCollection = utils::ErrorsCollection;

  terra::World LoadWorld(std::string_view filename);
  /*
  terra::World NewWorld(NewWorldParameters params, const ruleset::RuleSet& active_rule_set);
  */
  void SaveWorld(const terra::World&, std::string_view filename);

  std::optional<ruleset::RuleSet> LoadRuleSet(const std::filesystem::path& path,
    ErrorsCollection& errors);
};

}
