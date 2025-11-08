extends Resource

## Class that incpasulates (tries to) logic what to highlight and how to
## highlight
class_name HighlighterInterface

class InputDescription:
	extends RefCounted
	var variables: Array[StringName]


func get_input_description() -> InputDescription:
	return InputDescription.new()


func get_color(input: Dictionary[StringName, Variant]) -> Color:
	return Color.WHITE
