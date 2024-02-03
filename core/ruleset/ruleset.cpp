#include "ruleset.hpp"

#include <fstream>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <spdlog/spdlog.h>

#include <region/improvements.pb.h>

namespace hs::ruleset {

bool RuleSet::Load(const std::filesystem::path& path,
  ErrorsCollection& errors) {
  // TODO: Skip files that are not present
  const auto improvements_path = path / improvements_file;

  {
    std::ifstream f(improvements_path);
    if (!f.is_open()) {
      // for now, just throw an exception
      errors.AddError(
        utils::ErrorMessage{fmt::format("Can't open file {}", improvements_path.c_str())});
      return false;
    }
    google::protobuf::io::IstreamInputStream pb_input_stream{&f};

    // TODO: Must actually merge them
    return google::protobuf::TextFormat::Parse(&pb_input_stream, &improvements_);
  }
}

void RuleSet::Clear() {
  improvements_.Clear();
}

}
