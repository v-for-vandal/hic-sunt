extends TileMap

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		print("Tilemap: Received input: ", event)
		#Input.parse_input_event(SurfaceInputEvent.new())
