extends GameTile

func _ready() -> void:
	super()
	# Get parent
	var surface := get_surface()
	
	# Get biome of a given cell
	var plane : PlaneObject = surface.get_plane().plane_object
	assert(plane != null)
	if plane != null:
		var region := plane.get_region(qr_coords)
		var topBiomes := region.get_string_value_topn(Modifiers.ECOSYSTEM_BIOME, 1)
		if topBiomes.is_empty():
			push_error("No biome at all in region at: ", qr_coords)
		else:
			var biome := topBiomes[0]
			# Get texture for this biome
			var texture := GfxRegistry.get_biome_texture(biome)
			$Biome.texture = texture
			
	# connect to debugging nodes
	if DebugRoot.is_debug_enabled():
		DebugRoot.get_debug_display_options().changed.connect(self.on_debug_display_settings_changed)
			
func _update_highlighting() -> void:
	var surface := self.get_surface()
	if surface == null:
		push_error("cell does not belong to any surface")
		return
	if surface.highlighter == null:
		$Highlight.visible = false
		return
		
	var plane : PlaneObject = surface.get_plane().plane_object
	assert(plane != null)
	if plane == null:
		$Highlight.visible = false
		return
		
	var region := plane.get_region(qr_coords)
	var highlighter := surface.highlighter
	var input_description := highlighter.get_input_description()
	var input : Dictionary[StringName, Variant] = {}
	for variable in input_description.variables:
		# Note: there is in fact no guarantee that this variable is numeric, we should
		# check it via Ruleset
		# TODO: Fix note above
		input[variable] = region.get_numeric_value_aggregates(variable).avg
		
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

func _on_mouse_entered() -> void:
	pass

func _on_mouse_exited() -> void:
	pass # Replace with function body.

func _on_display_settings_changed() -> void:
	_update_highlighting()

func on_debug_display_settings_changed(options: DebugDisplayOptions) -> void:
	_update_debug_display_variable_or_modifier(options)
	
	

	
