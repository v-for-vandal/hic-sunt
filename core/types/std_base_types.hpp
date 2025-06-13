#pragma once

#include <string>

namespace hs {

    class StdBaseTypes {
    public:
        // TODO: use boost flyweight ?
        using StringId = std::string;
        using StringIdRef = const std::string&;

        using Strigng = std::string;
        using StringRef = const std::string&;
    };
}
