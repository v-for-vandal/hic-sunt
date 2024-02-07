extends TileMap

signal cell_clicked(region_object: RegionObject, qr_position: Vector2i)

var _region_object

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



func _unhandled_input(event) -> void:
	if event is InputEventMouseButton:
		if event.pressed:
			#print("Pressed button. event position: ", event.position, " in local: ", to_local(event.position))
			
			var tile_xy = local_to_map(to_local(event.position))
			var tile_qr = QrsCoordsLibrary.xy_to_qrs(tile_xy)
			print("RegionSurface: Cell coords xy:", tile_xy, " qr:", tile_qr)
			cell_clicked.emit(_region_object, tile_qr)



func load_region(region_object : RegionObject):
	assert(region_object != null, "Can't load nullptr as region")
	clear()
	
	_region_object = region_object
	
	print("type of region object:", type_string(typeof(region_object)))
	
	# dimensions are in (q,r,s) system with s omited
	# tilemap is in (x,y) system
	var qr_dimensions = region_object.get_dimensions()
	print("region dimensions: ", qr_dimensions)
	
	var q_len = qr_dimensions.x
	var r_len = qr_dimensions.y

	for q in q_len:
		for r in r_len:

			var terrain = region_object.get_cell_info(Vector2i(q,r))["terrain"]
			#print("Terrain of a cell qr=", Vector2i(q,r), " is \"", terrain, "\"")
			# convert to xy dimensions
			var xy_coords = QrsCoordsLibrary.qrs_to_xy(Vector2i(q,r))
			# fill cell
			if terrain_mapping.has(terrain):
				print("setting terrail of tile map xy=", xy_coords, " to ", terrain_mapping[terrain])
				set_cell(0, xy_coords, 0, terrain_mapping[terrain],0)
