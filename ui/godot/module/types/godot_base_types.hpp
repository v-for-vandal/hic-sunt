#pragma once

#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/string.hpp>

#include <ui/godot/module/utils/fmt.hpp>

namespace hs::godot {

    class GodotBaseTypes {
    public:
        using StringId = ::godot::StringName;

        using String = ::godot::String;

        static bool IsNullToken(const auto& string) noexcept {
            return string.length() == 0;
        }

        // This method should only be used to serialize godod Strings
        // into protobuf. For general conversion to std::string see
        // ui/godot/module/utils/to_string.hpp
        static auto ToProtoString(const ::godot::StringName& input) noexcept {
            return ToProtoString(::godot::String(input));
        }
        // This method should only be used to serialize godod Strings
        // into protobuf. For general conversion to std::string see
        // ui/godot/module/utils/to_string.hpp
        static std::string ToProtoString(const ::godot::String& input) noexcept {
            auto utf8str = input.utf8();
            return std::string{utf8str.ptr(), static_cast<size_t>(utf8str.size())};
        }
    };

}
