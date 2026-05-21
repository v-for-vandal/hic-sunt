extends Node

class_name GameTile

var _qr_coords : Vector2i
var _offset_coords: Vector2i

## QRS coords are coordinates in parent region/plane and are used
## by underlying C++ module
var qr_coords: Vector2i:
	get:
		return _qr_coords
		
## Offset coords are coodinates of this scene in TileMapLayer
## and are used when interacting with TileMapLayer
var offset_coords: Vector2i:
	get:
		return _offset_coords

signal input_event(tile_qr: Vector2i, event: InputEvent)


func _ready() -> void:
	# Get parent
	var surface := get_surface()
	assert(surface != null)
	_offset_coords = (get_parent() as TileMapLayer).local_to_map(self.position)
	_qr_coords = surface.map_to_axial(_offset_coords)
	input_event.connect(surface._on_input_event_from_cell)
	surface._display_settings_changed.connect(self._on_display_settings_changed)


## This method relies on the fact that parent TileMapLayer
## is direct descendant from GameTileSurface
## See https://docs.godotengine.org/en/stable/classes/class_tilesetscenescollectionsource.html#class-tilesetscenescollectionsource
func get_surface() -> GameTileSurface:
	var parent := self.get_parent()
	assert(parent != null)
	var p_parent := parent.get_parent() as GameTileSurface
	assert(p_parent != null)
	return p_parent
	

func get_offset_coordinates() -> Vector2i:
	return _offset_coords 

		
func _get_qr_coordinates() -> Vector2i:
	return  _qr_coords
	
func _on_input_event(_viewport: Node, event: InputEvent, shape_idx: int) -> void:
	input_event.emit(qr_coords, event)
	
func _on_display_settings_changed() -> void:
	pass
	
## This helper method can be called by descendants to update highlighting.
## It accepts as input:
## 1. Polygon2D - method will change texture on this node
## 2. variable_getter - callable that we will call to get value of a specific variable
func do_update_highlighting(highlight_target: Polygon2D, variable_getter: Callable) -> void:
	if highlight_target == null:
		push_error("highlight target is null")
		return
	var surface := self.get_surface()
	if surface == null:
		push_error("cell does not belong to any surface")
		highlight_target.visible = false
		return
	if surface.highlighter == null:
		highlight_target.visible = false
		return

	var highlighter := surface.highlighter
	var input_description := highlighter.get_input_description()
	var input : Dictionary[StringName, Variant] = {}
	for variable in input_description.variables:
		# Note: there is in fact no guarantee that this variable is numeric, we should
		# check it via Ruleset
		# TODO: Fix note above
		input[variable] = variable_getter.call(variable).avg
		
	highlight_target.modulate = surface.highlighter.get_color(input)
	highlight_target.visible = true
	
func do_update_debug_display_variable_or_modifier(
	scope_var_display: Label,
	variable_getter: Callable,
	options: DebugDisplayOptions) -> void:
	if options.target_variable_modifier_on_cell == null or not options.display_selected_variable_modifier_on_cell:
		scope_var_display.visible = false
		return
		
	var target_variable := options.target_variable_modifier_on_cell.variable
	if target_variable.is_empty():
		scope_var_display.visible = false
		return
		
	var varvalue : Variant = variable_getter.call(target_variable)
	if varvalue is int or varvalue is float:
		scope_var_display.text = "%f" % snapped(varvalue, 0.01)
	else:
		scope_var_display.text = "%s" % varvalue
		
		
	scope_var_display.visible = true
	
func _update_debug_display_variable_or_modifier(options: DebugDisplayOptions) -> void:
	if options.target_variable_modifier_on_cell == null or not options.display_selected_variable_modifier_on_cell:
		$ScopeVarDisplay.visible = false
		return
		
	var target_variable := options.target_variable_modifier_on_cell.variable
	if target_variable.is_empty():
		$ScopeVarDisplay.visible = false
		return

	var surface := self.get_surface()
	if surface == null:
		push_error("cell does not belong to any surface")
		return
		
	var plane : PlaneObject = surface.get_plane().plane_object
	assert(plane != null)
	if plane == null:
		$ScopeVarDisplay.visible = false
		return
		
	var region := plane.get_region(qr_coords)
	if region.get_scope().is_string_variable(target_variable):
		var value := region.get_scope().get_string_value(target_variable)
		$ScopeVarDisplay.text = "%s" % value
	else:
		var value := region.get_scope().get_numeric_value(target_variable)
		$ScopeVarDisplay.text = "%f" % value
		
	$ScopeVarDisplay.visible = true
	
