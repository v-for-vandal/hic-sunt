extends GameTile

func _ready() -> void:
	super()
	# Get parent
	var surface := get_surface()
	assert(surface != null)	
	# Get biome of a given cell
	var region : RegionObject = surface.get_region()
	assert(region != null)
	if region != null:
		var cell := region.get_cell(qr_coords)
		var biome := cell.get_scope().get_string_value(Modifiers.ECOSYSTEM_BIOME)
		# Get texture for this biome
		var texture := GfxRegistry.get_biome_texture(biome)
		$Biome.texture = texture
	_update_highlighting()
	
	# connect to debugging nodes
	if DebugRoot.is_debug_enabled():
		DebugRoot.get_debug_display_options().changed.connect(self.on_debug_display_settings_changed)

		
func _update_highlighting() -> void:
	var surface: GameTileSurface = get_surface()
	if surface.highlighter == null:
		$Highlight.visible = false
		return
	var region : RegionObject = surface.get_region()
	assert(region != null)
	if region == null:
		$Highlight.visible = false
		return
		
	var cell := region.get_cell(qr_coords)
	var highlighter := surface.highlighter
	var input_description = highlighter.get_input_description()
	var input : Dictionary[StringName, Variant] = {}
	for variable in input_description.variables:
		# Note: there is in fact no guarantee that this variable is numeric, we should
		# check it via Ruleset
		# TODO: Fix note above
		input[variable] = cell.get_scope().get_numeric_value(variable)
		
	$Highlight.modulate = surface.highlighter.get_color(input)
	$Highlight.visible = true
		
	
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
		
	var region : RegionObject = surface.get_region()
	assert(region != null)
	if region == null:
		$ScopeVarDisplay.visible = false
		return
		
	var cell := region.get_cell(qr_coords)
	if cell == null:
		$ScopeVarDisplay.visible = false
		return
		
	if region.get_scope().is_string_variable(target_variable):
		var value := cell.get_scope().get_string_value(target_variable)
		$ScopeVarDisplay.text = "%s" % value
	else:
		var value := cell.get_scope().get_numeric_value(target_variable)
		$ScopeVarDisplay.text = "%d" % value
		
	$ScopeVarDisplay.visible = true


func _on_mouse_entered() -> void:
	pass # Replace with function body.


func _on_mouse_exited() -> void:
	pass # Replace with function body.

func _on_display_settings_changed():
	_update_highlighting()
	
func on_debug_display_settings_changed(options: DebugDisplayOptions) -> void:
	_update_debug_display_variable_or_modifier(options)
