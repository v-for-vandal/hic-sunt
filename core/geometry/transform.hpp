#pragma once

namespace hs::geometry {

    enum class Rotation {
      kNoRotation,
      kRotation90,
      kRotation180,
      kRotation270,

      kSize
    };

    enum class kMirror {
      kNoMirror,
      kMirror,
      kSize
    };

    struct Transform {
      Rotation rotation{Rotation::kNoRotation};
      Mirror mirror{Rotation::kNoMirror};
    };

}

