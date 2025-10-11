#include "ruleset_base.hpp"

#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <ruleset/region_improvements.pb.h>
#include <spdlog/spdlog.h>

#include <fstream>

namespace hs::ruleset {

namespace {

bool ReadFromFile(const std::filesystem::path &path, auto &proto_object,
                  utils::ErrorsCollection &errors) {
  {
    std::ifstream f(path);
    if (!f.is_open()) {
      // for now, just throw an exception
      errors.AddError(
          utils::ErrorMessage{fmt::format("Can't open file {}", path.c_str())});
      return false;
    }
    google::protobuf::io::IstreamInputStream pb_input_stream{&f};

    return google::protobuf::TextFormat::Parse(&pb_input_stream, &proto_object);
  }
}

}  // namespace

bool RuleSetBase::Load(const std::filesystem::path &path,
                       ErrorsCollection &errors) {
  // TODO: Skip files that are not present
  // TODO: Must actually merge rules, not overwrite them
  SPDLOG_INFO("Parsing improvements file");
  bool success = ReadFromFile(path / improvements_file, improvements_, errors);
  if (!success) {
    return false;
  }

  SPDLOG_INFO("Parsing biomes file");
  success = ReadFromFile(path / biomes_file, biomes_, errors);
  if (!success) {
    return false;
  }

  SPDLOG_INFO("Parsing resources file");
  success = ReadFromFile(path / resources_file, resources_, errors);
  if (!success) {
    return false;
  }

  SPDLOG_INFO("Parsing jobs file");
  success = ReadFromFile(path / jobs_file, jobs_, errors);
  if (!success) {
    return false;
  }

  SPDLOG_INFO("Parsing rendering file");
  success = ReadFromFile(path / rendering_file, rendering_, errors);
  if (!success) {
    return false;
  }

  SPDLOG_INFO("Parsing projects file");
  success = ReadFromFile(path / projects_file, projects_, errors);
  if (!success) {
    return false;
  }

  return true;
}

void RuleSetBase::Clear() {
  biomes_.Clear();
  improvements_.Clear();
  resources_.Clear();
  rendering_.Clear();
  jobs_.Clear();
  projects_.Clear();
}

}  // namespace hs::ruleset
