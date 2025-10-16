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
		
	
	


func _on_mouse_entered() -> void:
	pass # Replace with function body.


func _on_mouse_exited() -> void:
	pass # Replace with function body.
