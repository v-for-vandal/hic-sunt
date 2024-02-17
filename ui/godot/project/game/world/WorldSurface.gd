extends GameTileSurface

#signal enter_region(world_object: WorldObject, qr_position: Vector2i)
#signal region_clicked(world_object: WorldObject, qr_position: Vector2i)

var _world_object : WorldObject

var terrain_mapping : Dictionary = {
	"coast" : Vector2i(0,0),
	"plains" : Vector2i(1,0),
	"sand" : Vector2i(2,0),
	"ocean" : Vector2i(3,0),
	"snow" : Vector2i(4,0),
	"core.coast" : Vector2i(0,0),
	"core.plains" : Vector2i(1,0),
	"core.sand" : Vector2i(2,0),
	"core.ocean" : Vector2i(3,0),
	"core.snow" : Vector2i(4,0),
}

func _ready():
	# add highlight layer
	add_layer(-1)
	_highlight_layer_id = get_layers_count() - 1
	
func _contains(tile_qr: Vector2i) -> bool:
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




func load_world(world_object : WorldObject):
	assert(world_object != null, "Can't load nullptr as world")
	clear()
	
	_world_object = world_object
	
	print("type of world object:", type_string(typeof(world_object)))
	
	# dimensions are in (q,r,s) system with s omited
	# tilemap is in (x,y) system
	var qr_dimensions = world_object.get_dimensions()
	print("world dimensions: ", qr_dimensions)
	
	var q_len = qr_dimensions.x
	var r_len = qr_dimensions.y

	for q in q_len:
		for r in r_len:
			var qr_coords = Vector2i(q,r)
			var region_info : Dictionary = world_object.get_region_info(qr_coords)
			if region_info.is_empty():
				print("Can't get region at: ", qr_coords)
				continue
			var top_terrain: Array = region_info.top_terrain
			var terrain = top_terrain[1]
			#print("Terrain of a cell qr=", Vector2i(q,r), " is \"", terrain, "\"")
			# convert to xy dimensions
			var xy_coords = QrsCoordsLibrary.qrs_to_xy(qr_coords)
			# fill cell
			if terrain_mapping.has(terrain):
				#print("setting terrail of tile map xy=", xy_coords, " qr=", qr_coords, " to ",
				#	terrain_mapping[terrain])
				set_cell(0, xy_coords, 0, terrain_mapping[terrain],0)
			else:
				print("unknown terrain", terrain)
