#pragma once

#include <flatbuffers/flatbuffers.h>

namespace hs::serialize {

  template<typename T>
  struct To {
    using Target = T;
  };

}
