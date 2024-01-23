#include "system.hpp"

#include <fstream>

#include <core/utils/serialize.hpp>


namespace hs::system {

std::shared_ptr<terra::World> System::LoadWorld(std::string_view filename) {
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

  return std::make_shared<terra::World>(ParseFrom(*fbs_world, serialize::To<terra::World>{}));

}

void System::SaveWorld(const terra::World& target, std::string_view filename) {
  ::flatbuffers::FlatBufferBuilder fbb{};
  auto offset = SerializeTo(target, fbb);
  fbb.Finish(offset);

  std::ofstream out(filename, std::ios::binary);
  out.write( (char*)fbb.GetBufferPointer(), fbb.GetSize());
  out.close();

}


}
