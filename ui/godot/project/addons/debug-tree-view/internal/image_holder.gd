extends TextureRect

var _zoom_tween: Tween

func _gui_input(event: InputEvent) -> void:
	var handled := false

	if event is InputEventMouseMotion and Input.is_mouse_button_pressed(MOUSE_BUTTON_LEFT):
		var move_event := event as InputEventMouseMotion
		if not move_event.relative.is_zero_approx():
			self.position += move_event.relative 
		handled = true

	if handled:
		get_viewport().set_input_as_handled()
