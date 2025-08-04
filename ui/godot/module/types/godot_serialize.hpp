#pragma once

#include <string>
#include <ui/godot/module/utils/to_string.hpp>
#include <core/utils/serialize.hpp>

namespace godot {

// This method should only be used to serialize godod Strings
// into protobuf. For general conversion to std::string see
// ui/godot/module/utils/to_string.hpp
inline void SerializeTo(const ::godot::StringName& source, std::string& target) noexcept {

    ::godot::String tmp{source};
    auto utf8str = tmp.utf8();
    target = std::string{utf8str.ptr(), static_cast<size_t>(utf8str.size())};
}


// This method should only be used to serialize godod Strings
// into protobuf. For general conversion to std::string see
// ui/godot/module/utils/to_string.hpp
inline void SerializeTo(const ::godot::String& source, std::string& target) noexcept {
            auto utf8str = source.utf8();
            target = std::string{utf8str.ptr(), static_cast<size_t>(utf8str.size())};
}

}

namespace hs::serialize {

// This method should only be used to serialize godod Strings
// into protobuf. For general conversion to std::string see
// ui/godot/module/utils/to_string.hpp
inline ::godot::StringName ParseFrom(const std::string& source, serialize::To<::godot::StringName>) noexcept {
    return hs::godot::utils::to_string_name(source);
}

// This method should only be used to serialize godod Strings
// into protobuf. For general conversion to std::string see
// ui/godot/module/utils/to_string.hpp
inline ::godot::String ParseFrom(const std::string& source, serialize::To<::godot::String>) noexcept {
    return hs::godot::utils::to_string(source);
}

}
