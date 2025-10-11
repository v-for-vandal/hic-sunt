#pragma once

#include <core/scope/scope.hpp>
#include <ui/godot/module/types/godot_base_types.hpp>

namespace hs::godot {
using Scope = hs::scope::Scope<GodotBaseTypes>;
using ScopePtr = hs::scope::ScopePtr<GodotBaseTypes>;
}  // namespace hs::godot
