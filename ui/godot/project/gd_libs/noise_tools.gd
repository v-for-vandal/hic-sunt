extends RefCounted

class_name NoiseToolsLibrary

## Accepts value (between -1 and 1) and transforms proportionally into value inside target_range.
static func  sample_simple_range_i(value: float, target_range : Vector2i) -> int:
	return int(float(target_range.x) + (target_range.y - target_range.x) * (value - (-1.0)) / 2.0)
