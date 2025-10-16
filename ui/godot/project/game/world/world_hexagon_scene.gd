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
		var topBiomes := region.get_topn_string_values(Modifiers.ECOSYSTEM_BIOME, 1)
		if topBiomes.is_empty():
			push_error("No biome at all in region at: ", qr_coords)
		else:
			var biome := topBiomes[0]
			# Get texture for this biome
			var texture := GfxRegistry.get_biome_texture(biome)
			$Biome.texture = texture

func _on_mouse_entered() -> void:
	pass

func _on_mouse_exited() -> void:
	pass # Replace with function body.
