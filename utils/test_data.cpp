#include "test_data.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <string>
#include <string_view>
#include <system_error>

namespace hs::test {
namespace {

std::filesystem::path GetTestResourcesRoot() {
  const char* root = std::getenv("TEST_RESOURCES_DIR");
  if (root == nullptr || std::string(root).empty()) {
    ADD_FAILURE() << "TEST_RESOURCES_DIR is not set. Unit tests with data folders must be "
                     "run through Meson so the test resources root is passed in the environment.";
    return {};
  }
  return std::filesystem::path(root);
}

bool IsSafeRelativePath(const std::filesystem::path& path) {
  if (path.empty() || path.is_absolute()) {
    return false;
  }
  for (const auto& part : path) {
    if (part == "..") {
      return false;
    }
  }
  return true;
}

std::filesystem::path RequireDirectory(const std::filesystem::path& path,
                                       std::string_view description) {
  std::error_code error;
  const auto status = std::filesystem::status(path, error);
  if (error) {
    ADD_FAILURE() << "Cannot inspect " << description << ": " << path << " (" << error.message()
                  << ")";
    return path;
  }
  if (!std::filesystem::exists(status)) {
    ADD_FAILURE() << "Missing " << description << ": " << path;
    return path;
  }
  if (!std::filesystem::is_directory(status)) {
    ADD_FAILURE() << "Expected " << description << " to be a directory, got: " << path;
  }
  return path;
}

std::filesystem::path CurrentTestDataFolder() {
  const auto* test_info = ::testing::UnitTest::GetInstance()->current_test_info();
  if (test_info == nullptr) {
    ADD_FAILURE() << "GetTestDataFolder() can only be called while a gtest test is running.";
    return {};
  }

  return GetTestResourcesRoot() / "tests" / test_info->test_suite_name() / test_info->name();
}

}  // namespace

std::filesystem::path GetTestDataFolder() {
  const auto path = CurrentTestDataFolder();
  return RequireDirectory(path, "test data folder");
}

std::filesystem::path GetTestDataFolder(const std::filesystem::path& subfolder_path) {
  if (!IsSafeRelativePath(subfolder_path)) {
    ADD_FAILURE() << "GetTestDataFolder(subfolder_path) expects a non-empty relative path without "
                     "'..' components, got: "
                  << subfolder_path;
    return {};
  }

  const auto path = CurrentTestDataFolder() / subfolder_path;
  return RequireDirectory(path, "test data subfolder");
}

std::filesystem::path GetCommonTestDataFolder(const std::filesystem::path& subfolder_path) {
  if (!IsSafeRelativePath(subfolder_path)) {
    ADD_FAILURE() << "GetCommonTestDataFolder(subfolder_path) requires a non-empty relative path "
                     "without '..' components, got: "
                  << subfolder_path;
    return {};
  }

  const auto path = GetTestResourcesRoot() / "common" / subfolder_path;
  return RequireDirectory(path, "common test data folder");
}

}  // namespace hs::test
