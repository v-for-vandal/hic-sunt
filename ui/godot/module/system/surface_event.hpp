#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/input_event.hpp>

using namespace godot;

class SurfaceInputEvent : public InputEvent {
  GDCLASS(SurfaceInputEvent, InputEvent);

public:
  SurfaceInputEvent() {}

  void _init() {
  }

  //void set_world(std::shared_ptr<terra::World> world_ptr);

  static void _bind_methods()
  {}
};
