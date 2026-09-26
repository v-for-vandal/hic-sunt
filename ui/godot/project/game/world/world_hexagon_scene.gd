extends GameTile

var _region: RegionObject

func _ready() -> void:
	super()
	_load_cell()
	
	CurrentGame.ui_city_created.connect(self.on_some_city_created)
	# connect to debugging nodes
	if DebugRoot.is_debug_enabled():
		DebugRoot.get_debug_display_options().changed.connect(self.on_debug_display_settings_changed)
	

func _load_cell() -> void:
	# Get parent
	var surface := get_surface()
	
	# Get biome of a given cell
	var plane : PlaneObject = surface.get_plane().plane_object
	assert(plane != null)
	if plane != null:
		_region = plane.get_region(qr_coords)
		assert(_region != null)
		var topBiomes := _region.get_string_value_topn(Modifiers.ECOSYSTEM_BIOME, 1)
		if topBiomes.is_empty():
			push_error("No biome at all in region at: ", qr_coords)
		else:
			var biome := topBiomes[0]
			# Get texture for this biome
			var texture := GfxRegistry.get_biome_texture(biome)
			$Biome.texture = texture
			
		var city := CurrentGame.game.world.find_city_by_region_id(_region.get_id())
		if city != null:
			# TODO: Improvement name is hardcoded, it should not be so. In fact,
			# we should use some UI element for that.
			var city_texture := GfxRegistry.get_improvement_texture("city.hall")
			$GlobalImprovement.texture = city_texture
			$GlobalImprovement.visible = true
		else:
			$GlobalImprovement.visible = false

			
func _update_highlighting() -> void:
	var surface := self.get_surface()
	if surface == null:
		push_error("cell does not belong to any surface")
		return
		
	var plane : PlaneObject = surface.get_plane().plane_object
	assert(plane != null)
	if plane == null:
		$Highlight.visible = false
		return
		
	var region := plane.get_region(qr_coords)
	
	var variable_getter := func(varname: StringName) -> Variant:
		# Note: there is in fact no guarantee that this variable is numeric, we should
		# check it via Ruleset
		# TODO: Fix note above
		return region.get_numeric_value_aggregates(varname).avg
		
	
	do_update_highlighting($Highlight, variable_getter)

	
func _update_debug_display_variable_or_modifier(options: DebugDisplayOptions) -> void:
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
	if region == null:
		$ScopeVarDisplay.visible = false
		return
		
		
	var variable_getter := func(varname: StringName) -> Variant:
		if region.get_scope().is_string_variable(varname):
			return region.get_scope().get_string_value(varname)
		else:
			return region.get_scope().get_numeric_value(varname)
			
	do_update_debug_display_variable_or_modifier($ScopeVarDisplay, variable_getter, options)

func _on_mouse_entered() -> void:
	pass

func _on_mouse_exited() -> void:
	pass # Replace with function body.

func _on_display_settings_changed() -> void:
	_update_highlighting()

func on_debug_display_settings_changed(options: DebugDisplayOptions) -> void:
	_update_debug_display_variable_or_modifier(options)
	
func on_some_city_created(city: City) -> void:
	if not _region:
		return
	if city.get_primary_region_id() == _region.get_id():
		_load_cell()
	
	

	
