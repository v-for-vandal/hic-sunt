#pragma once

#include <core/terra/plane.hpp>
#include <ui/godot/module/types/godot_base_types.hpp>

namespace hs::godot {

    using Plane = hs::terra::Plane<GodotBaseTypes>;
    using PlanePtr = hs::terra::PlanePtr<GodotBaseTypes>;

}

