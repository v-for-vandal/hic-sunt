#pragma once

#include <filesystem>

namespace hs::test {

std::filesystem::path GetTestDataFolder();
std::filesystem::path GetTestDataFolder(const std::filesystem::path& subfolder_path);

std::filesystem::path GetCommonTestDataFolder(const std::filesystem::path& subfolder_path);

}  // namespace hs::test
