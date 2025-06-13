#pragma once

#include <string>

namespace hs {

    class StdBaseTypes {
    public:
        // TODO: use boost flyweight ?
        using StringId = std::string;
        using StringIdRef = const std::string&;

        using String = std::string;
        using StringRef = const std::string&;

        bool IsNullToken(const auto& string) {
            return string.size() == 0;
        }
    };
}
