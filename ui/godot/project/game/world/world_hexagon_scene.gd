extends GameTile

func _ready() -> void:
	super()
	# Get parent
	var surface := get_surface()
	
	# Get biome of a given cell
	var plane : PlaneObject = surface.get_plane()
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
			
func _update_highlighting(surface: GameTileSurface) -> void:
	if surface.highlighter == null:
		$Highlight.visible = false
		return
	var plane : PlaneObject = surface.get_plane()
	assert(plane != null)
	if plane == null:
		$Highlight.visible = false
		return
		
	var region := plane.get_region(qr_coords)
	var highlighter := surface.highlighter
	var input_description = highlighter.get_input_description()
	var input : Dictionary[StringName, Variant] = {}
	for variable in input_description.variables:
		# Note: there is in fact no guarantee that this variable is numeric, we should
		# check it via Ruleset
		# TODO: Fix note above
		input[variable] = region.get_numeric_value_aggregates(variable).avg
		
	$Highlight.modulate = surface.highlighter.get_color(input)
	$Highlight.visible = true

func _on_mouse_entered() -> void:
	pass

func _on_mouse_exited() -> void:
	pass # Replace with function body.

func _on_display_settings_changed(surface: GameTileSurface):
	_update_highlighting(surface)
