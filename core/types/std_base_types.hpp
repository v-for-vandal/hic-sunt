#pragma once

#include <string>

namespace hs {

    class StdBaseTypes {
    public:
        // TODO: use boost flyweight ?
        using StringId = std::string;

        using String = std::string;

        static bool IsNullToken(const auto& string) noexcept {
            return string.size() == 0;
        }

        template<typename T>
        static auto ToProtoString(T&& input) noexcept {
            return std::forward<T>(input);
        }
    };
}
