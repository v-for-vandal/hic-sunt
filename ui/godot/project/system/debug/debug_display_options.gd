extends RefCounted

class_name DebugDisplayOptions

## This class holds options for what to display and what not to display
## when in debug mode

signal changed(options: DebugDisplayOptions)

## When set to true, an overlay over every cell will display a number
## with value of given variables/modifier
# TODO: Should be false by default
var display_selected_variable_modifier_on_cell := true:
	set(value):
		display_selected_variable_modifier_on_cell = value
		changed.emit(self)

## Target modifier/variable to display. See
## display_selected_variable_modifier_on_cell
var target_variable_modifier_on_cell : String = "":
	set(value):
		target_variable_modifier_on_cell = value
		changed.emit(self)
