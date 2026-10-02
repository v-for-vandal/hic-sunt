#include <core/ruleset/ruleset.hpp>
#include <core/types/std_base_types.hpp>
#include <core/utils/error_message.hpp>
#include <filesystem>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <ruleset-root> [<ruleset-root> ...]\n";
    return 1;
  }

  std::vector<std::filesystem::path> roots;
  roots.reserve(static_cast<size_t>(argc - 1));
  for (int idx = 1; idx < argc; ++idx) {
    roots.emplace_back(argv[idx]);
  }

  hs::utils::ErrorsCollection errors;
  hs::ruleset::RuleSet<hs::StdBaseTypes> ruleset;
  const bool success = ruleset.Load(roots, errors);

  for (const auto& error : errors.errors) {
    std::cerr << error.message << '\n';
  }

  return success ? 0 : 1;
}
