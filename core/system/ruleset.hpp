#pragma once

#include <string_view>
#include <filesystem>

namespace hs::system {

class RuleSet {
public:
  // Adds data to ruleset
  void Load(std::string_view path);
};

}
