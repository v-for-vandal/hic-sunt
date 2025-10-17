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
	_update_highlighting(surface)

		
func _update_highlighting(surface: GameTileSurface) -> void:
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
		
	
	


func _on_mouse_entered() -> void:
	pass # Replace with function body.


func _on_mouse_exited() -> void:
	pass # Replace with function body.

func _on_display_settings_changed(surface: GameTileSurface):
	_update_highlighting(surface)
