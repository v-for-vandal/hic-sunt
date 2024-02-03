#pragma once

namespace hs::utils {
  struct ErrorMessage {
    std::string message;
  };

  struct ErrorsCollection {
    std::vector<ErrorMessage> errors;

    void AddError(ErrorMessage msg) {
      errors.emplace_back(std::move(msg));
    }
  };


}
