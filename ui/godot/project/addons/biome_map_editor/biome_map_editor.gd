@tool
extends EditorInspectorPlugin

func _can_handle(object: Object) -> bool:
	return object is BiomeMap

func _parse_begin(object: Object) -> void:
	var button = Button.new()
	button.text = "Some text"
	add_custom_control(button)
