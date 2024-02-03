#pragma once

#include <memory>
#include <filesystem>

#include <core/ruleset/ruleset.hpp>
#include <core/terra/world.hpp>

namespace hs::system {

struct NewWorldParameters {
  terra::World::QRSCoordinateSystem::QDelta q_size{0};
  terra::World::QRSCoordinateSystem::RDelta r_size{0};
};

/* Central component for game */
class System {
public:
  using ErrorsCollection = utils::ErrorsCollection;

  terra::World LoadWorld(std::string_view filename);
  terra::World NewWorld(NewWorldParameters params);
  void SaveWorld(const terra::World&, std::string_view filename);

  bool LoadRuleSet(const std::filesystem::path& path,
    ErrorsCollection& errors);


private:
  ruleset::RuleSet active_rule_set_;

};

}
