extends GameTileSurface

#signal enter_region(world_object: WorldObject, qr_position: Vector2i)
#signal region_clicked(world_object: WorldObject, qr_position: Vector2i)

var _world_object : WorldObject

var terrain_mapping : Dictionary = {}

func _ready() -> void:
	pass
	
func _contains(tile_qr: Vector2i) -> bool:
	if _world_object == null:
		push_error("WorldSurface is running in uninitialized mode")
		return false

	assert(_world_object != null)
	return _world_object.contains(tile_qr)
	

#func _unhandled_input(event) -> void:
	#if event is InputEventMouseButton:
		## react on mouse release, not on mouse press
		#if not event.pressed and not event.canceled:
			#event = make_input_local(event)
			#var tile_xy := local_to_map(to_local(event.position))
			#var tile_qr := QrsCoordsLibrary.xy_to_qrs(tile_xy)
			#print("WorldSurface: Cell coords xy:", tile_xy, " qr:", tile_qr)
			#if _world_object.contains(tile_qr):
				#GameUiEventBus.emit_world_cell_action(tile_qr,
				#GameUiEventBus.mouse_button_to_action_type(event.button_index))




func load_world(world_object : WorldObject) -> void:
	assert(world_object != null, "Can't load nullptr as world")
	clear()
	
	_world_object = world_object
	
	print("type of world object:", type_string(typeof(world_object)))
	
	# dimensions are in (q,r,s) system with s omited
	# tilemap is in (x,y) system
	var qr_dimensions : Rect2i = world_object.get_dimensions()
	print("world dimensions: ", qr_dimensions)

	for q in range(qr_dimensions.position.x, qr_dimensions.end.x):
		for r in range(qr_dimensions.position.y, qr_dimensions.end.y):

			var qr_coords := Vector2i(q,r)
			if not world_object.contains(qr_coords):
				print("skipping non-existing world cell ", qr_coords)
				continue
			var region_info : Dictionary = world_object.get_region_info(qr_coords)
			if region_info.is_empty():
				push_error("Can't get region at: ", qr_coords)
				continue
				
			var top_terrain: Array = region_info.top_biome
			var terrain : String = top_terrain[1]
			var has_city : bool = not region_info.city_id.is_empty()
			#print("Terrain of a cell qr=", Vector2i(q,r), " is \"", terrain, "\"")
			# convert to xy dimensions
			var xy_coords := QrsCoordsLibrary.qrs_to_xy(qr_coords)
			# fill cell
			if has_city:
				$terrain.set_cell(xy_coords, 0, Vector2i(0,2), 0)
			elif terrain_mapping.has(terrain):
				#print("setting terrail of tile map xy=", xy_coords, " qr=", qr_coords, " to ",
				#	terrain_mapping[terrain])
				$terrain.set_cell( xy_coords, terrain_mapping[terrain].source_id, terrain_mapping[terrain].atlas_coords,0)
			else:
				push_error("unknown terrain \"%s\"" % [terrain])
				$terrain.set_cell(xy_coords, 0, Vector2i(0,0), 0)
