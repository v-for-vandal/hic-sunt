extends TileMap

signal cell_clicked(world_object: WorldObject, qr_position: Vector2i)

var _world_object

var terrain_mapping : Dictionary = {
	"coast" : Vector2i(0,0),
	"plains" : Vector2i(1,0),
	"sand" : Vector2i(2,0),
	"ocean" : Vector2i(3,0),
	"snow" : Vector2i(4,0),
}



func _unhandled_input(event) -> void:
	if event is InputEventMouseButton:
		if event.pressed:
			#print("Pressed button. event position: ", event.position, " in local: ", to_local(event.position))
			
			var tile_xy = local_to_map(to_local(event.position))
			var tile_qr = QrsCoordsLibrary.xy_to_qrs(tile_xy)
			print("Cell coords xy:", tile_xy, " qr:", tile_qr)
			cell_clicked.emit(_world_object, tile_qr)



func load_world(world_object : WorldObject):
	assert(world_object != null, "Can't load nullptr as world")
	clear()
	
	_world_object = world_object
	
	print("type of world object:", type_string(typeof(world_object)))
	print("improvements:" , world_object.get_region_improvements())
	
	# dimensions are in (q,r,s) system with s omited
	# tilemap is in (x,y) system
	var qr_dimensions = world_object.get_dimensions()
	print("world dimensions: ", qr_dimensions)
	
	var q_len = qr_dimensions.x
	var r_len = qr_dimensions.y

	for q in q_len:
		for r in r_len:

			var terrain = world_object.get_cell_terrain(Vector2i(q,r))
			#print("Terrain of a cell qr=", Vector2i(q,r), " is \"", terrain, "\"")
			# convert to xy dimensions
			var xy_coords = QrsCoordsLibrary.qrs_to_xy(Vector2i(q,r))
			# fill cell
			if terrain_mapping.has(terrain):
				#print("setting terrail of tile map xy=", xy_coords, " to ", terrain_mapping[terrain])
				set_cell(0, xy_coords, 0, terrain_mapping[terrain],0)
