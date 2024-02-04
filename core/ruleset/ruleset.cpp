#include "ruleset.hpp"

#include <fstream>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <spdlog/spdlog.h>

#include <ruleset/region_improvements.pb.h>

namespace hs::ruleset {

namespace {

bool ReadFromFile(const std::filesystem::path& path, auto& proto_object, utils::ErrorsCollection& errors) {
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

}

bool RuleSet::Load(const std::filesystem::path& path,
  ErrorsCollection& errors) {
  // TODO: Skip files that are not present
    // TODO: Must actually merge rules, not overwrite them
  bool success = ReadFromFile( path / improvements_file, improvements_, errors);
  if(!success) {
    return false;
  }

  success = ReadFromFile( path / terrain_file, terrain_, errors);
  if(!success) {
    return false;
  }

}

void RuleSet::Clear() {
  improvements_.Clear();
}

}
