extends HighlighterInterface

class_name NumericVariableHighlighter

@export
var gradient : Gradient

@export
var variable_name: String

## What range should be mapped to color gradient. Values outside of this range
## are allowed, but are always mapped to boundaries.
@export
var range: Vector2i

func get_input_description() -> InputDescription:
	var result := HighlighterInterface.InputDescription.new()
	result.variables.append(variable_name)
	return result
	
func get_color(input: Dictionary[StringName, Variant]) -> Color:
	# We could get range from ruleset, but for now we stick to some hardcoded constants
	var clamped := clampf(input[variable_name] as float, range.x, range.y)
	var target := float(clamped - range.x + 1) / (range.y - range.x + 1) # +1 to avoid zero division
	return gradient.sample(target)
