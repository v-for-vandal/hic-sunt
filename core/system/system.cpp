#include "system.hpp"

#include <fstream>

#include <core/utils/serialize.hpp>


namespace hs::system {

terra::World System::LoadWorld(std::string_view filename) {
  /* TODO: Implement reading from mmap file */
  /*
  int fd = open(filename, O_RDONLY);

  */
  std::ifstream in(filename, std::ios::binary);
  std::vector<char> buffer{ std::istreambuf_iterator<char>(in),
                      std::istreambuf_iterator<char>()};

  const fbs::World* fbs_world = fbs::GetWorld(buffer.data());
  if(fbs_world == nullptr) {
    return {};
  }

  return ParseFrom(*fbs_world, serialize::To<terra::World>{});

}


terra::World System::NewWorld(NewWorldParameters params) {
  static std::array terrain_types = { "coast", "plains", "sand", "ocean", "snow" };

  auto result = terra::World{params.q_size, params.r_size};
  auto surface = result.GetSurface();
  /* randomize terrain for now */
  for(auto q_pos = surface.q_start(); q_pos < surface.q_end(); q_pos++) {
    for( auto r_pos = surface.r_start(); r_pos < surface.r_end(); r_pos++) {
      auto coords = terra::World::QRSCoords{q_pos, r_pos};
      if(surface.Contains(coords)) {
        surface.GetCell(coords).SetTerrain( terrain_types[ std::rand() % terrain_types.size() ] );
      }
    }
  }

  result.SetRuleSet(active_rule_set_);

  return result;

}

void System::SaveWorld(const terra::World& target, std::string_view filename) {
  ::flatbuffers::FlatBufferBuilder fbb{};
  auto offset = SerializeTo(target, fbb);
  fbb.Finish(offset);

  std::ofstream out(filename, std::ios::binary);
  out.write( (char*)fbb.GetBufferPointer(), fbb.GetSize());
  out.close();

}

bool System::LoadRuleSet(const std::filesystem::path& path,
  ErrorsCollection& errors) {
  const bool result = active_rule_set_.Load(path, errors);
  return result;
}


}
