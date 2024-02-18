#include "system.hpp"

#include <fstream>
#include <vector>

#include <core/utils/serialize.hpp>


namespace hs::system {

terra::World System::LoadWorld(std::string_view filename) {
  /* TODO: Implement reading from mmap file */
  /*
  int fd = open(filename, O_RDONLY);

  */
  std::ifstream in(filename, std::ios::binary);

  proto::terra::World proto_world;
  proto_world.ParseFromIstream(&in);

  return ParseFrom(proto_world, serialize::To<terra::World>{});

}


terra::World System::NewWorld(NewWorldParameters params, const ruleset::RuleSet& active_rule_set) {
  std::vector<std::string> terrain_types;
  for(const auto& terrain_type: active_rule_set.GetTerrain().terrain_types()) {
    terrain_types.push_back(terrain_type.id());
  }

  auto randomize_region = [&terrain_types](region::Region& region) {
    for(auto q_pos = region.GetSurface().q_start(); q_pos < region.GetSurface().q_end(); q_pos++) {
      for( auto r_pos = region.GetSurface().r_start(); r_pos < region.GetSurface().r_end(); r_pos++) {
        auto coords = terra::World::QRSCoords{q_pos, r_pos};
        if(region.GetSurface().Contains(coords)) {
          region.SetTerrain( coords, terrain_types[ std::rand() % terrain_types.size() ] );
        }
      }
    }
  };

  auto result = terra::World{
      terra::World::QRSCoordinateSystem::QAxis{0},
      terra::World::QRSCoordinateSystem::QAxis{0} + params.world_size.q(),
      terra::World::QRSCoordinateSystem::RAxis{0},
      terra::World::QRSCoordinateSystem::RAxis{0} + params.world_size.r(),
      terra::World::QRSCoordinateSystem::SAxis{0},
      terra::World::QRSCoordinateSystem::SAxis{0} + params.world_size.s()};

  auto surface = result.GetSurface();
  /* randomize terrain for now */
  for(auto q_pos = surface.q_start(); q_pos < surface.q_end(); q_pos++) {
    for( auto r_pos = surface.r_start(); r_pos < surface.r_end(); r_pos++) {
      auto coords = terra::World::QRSCoords{q_pos, r_pos};
      if(surface.Contains(coords)) {
        // initialize region
        region::Region region{params.region_size};
        randomize_region(region);
        result.SetRegion(coords, std::move(region));
      }
    }
  }

  return result;

}

void System::SaveWorld(const terra::World& target, std::string_view filename) {
  proto::terra::World proto_world;
  SerializeTo(target, proto_world);

  std::ofstream out(filename, std::ios::binary);
  proto_world.SerializeToOstream(&out);
  out.close();
}

std::optional<ruleset::RuleSet> System::LoadRuleSet(const std::filesystem::path& path,
  ErrorsCollection& errors) {
  ruleset::RuleSet result;
  const bool success = result.Load(path, errors);
  if(!success) {
    return std::nullopt;
  } else {
    return std::move(result);
  }
}


}
