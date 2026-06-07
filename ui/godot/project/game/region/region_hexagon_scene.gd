extends GameTile

var _cell_id : StringName

func _ready() -> void:
	super()
	_load_cell()

	# connect to global cell changed signal
	CurrentGame.ui_cell_changed.connect(_on_some_cell_changed)
	
	# connect to debugging nodes
	if DebugRoot.is_debug_enabled():
		DebugRoot.get_debug_display_options().changed.connect(self.on_debug_display_settings_changed)
	
func _load_cell() -> void:
	# Get parent
	var surface := get_surface()
	assert(surface != null)	
	# Get biome of a given cell
	var region : RegionObject = surface.get_region()
	assert(region != null)
	if region != null:
		var cell := region.get_cell(qr_coords)
		_cell_id = cell.get_id()
		var biome := cell.get_scope().get_string_value(Modifiers.ECOSYSTEM_BIOME)
		# Get texture for this biome
		var texture := GfxRegistry.get_biome_texture(biome)
		$Biome.texture = texture
		
		var improvement_scope : ScopeObject = cell.get_improvement(0) # slot 0
		if improvement_scope:
			var improvement_class := improvement_scope.get_string_value(WorldConstants.CLASS_VARIABLE)
			$Improvement0.texture = GfxRegistry.get_improvement_texture(improvement_class)
			$Improvement0.visible = true
	_update_highlighting()


		
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
	var variable_getter := func(varname: StringName) -> Variant:
		return cell.get_scope().get_variant_value(varname)
		
	do_update_highlighting($Highlight, variable_getter)
		
func _update_debug_display_variable_or_modifier(options: DebugDisplayOptions) -> void:
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
		
	var variable_getter := func(varname: StringName) -> Variant:
		if cell.get_scope().is_string_variable(varname):
			return cell.get_scope().get_string_value(varname)
		else:
			return cell.get_scope().get_numeric_value(varname)
		
	do_update_debug_display_variable_or_modifier($ScopeVarDisplay, variable_getter, options)



func _on_mouse_entered() -> void:
	pass # Replace with function body.


func _on_mouse_exited() -> void:
	pass # Replace with function body.
	
# This method will be called when some cell has changed. Not necesserilly our cell
func _on_some_cell_changed(cell: CellObject) -> void:
	if cell == null:
		return
		
	if cell.get_id() != _cell_id:
		return
		
	# Otherwise reload self
	_load_cell()

func _on_display_settings_changed() -> void:
	_update_highlighting()
	
func on_debug_display_settings_changed(options: DebugDisplayOptions) -> void:
	_update_debug_display_variable_or_modifier(options)
