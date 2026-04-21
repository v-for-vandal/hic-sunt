extends Camera2D

const MIN_ZOOM_FACTOR := Vector2(0.7, 0.7)
const ZOOM_PERCENTAGE := 0.15
const ZOOM_SPEED := 0.3

var _zoom_tween: Tween


func _input(event: InputEvent) -> void:
	var handled := false

	var change_zoom := false
	var new_zoom: Vector2
	var zoom_factor: = Vector2(
		max(MIN_ZOOM_FACTOR.x, zoom.x * ZOOM_PERCENTAGE),
		max(MIN_ZOOM_FACTOR.y, zoom.y * ZOOM_PERCENTAGE),
	)

	if event.is_action_pressed("ui_zoom_in"):
		change_zoom = true
		new_zoom = zoom + zoom_factor
	elif event.is_action_pressed("ui_zoom_out"):
		change_zoom = true
		new_zoom = zoom - zoom_factor

	if change_zoom:
		if _zoom_tween:
			_zoom_tween.kill()

		_zoom_tween = create_tween()
		_zoom_tween.tween_property(self, "zoom", new_zoom, ZOOM_SPEED)
		handled = true

	if event is InputEventMouseMotion and Input.is_mouse_button_pressed(MOUSE_BUTTON_LEFT):
		var move_event := event as InputEventMouseMotion
		if not move_event.relative.is_zero_approx():
			# invert displacement
			self.position -= move_event.relative / zoom
		handled = true

	if handled:
		get_viewport().set_input_as_handled()
