extends RefCounted

class_name NoiseToolsLibrary

# assumes that value is between -1 and 1.
static func  sample_simple_range_i(value: float, range : Vector2i) -> int:
	return range.x + (range.y - range.x) * (value - (-1.0)) / 2
