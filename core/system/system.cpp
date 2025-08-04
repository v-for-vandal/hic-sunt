#include "system.hpp"

#include <fstream>
#include <vector>

#include <core/utils/serialize.hpp>


namespace hs::system {

#if 0

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

#endif


}
