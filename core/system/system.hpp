#pragma once

#include <memory>

#include <core/terra/world.hpp>

namespace hs::system {

/* Central component for game */
class System {
public:
  std::shared_ptr<terra::World> LoadWorld(std::string_view filename);
  void SaveWorld(const terra::World&, std::string_view filename);

private:

};

}
