extends GameTileSurface

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

func _ready() -> void:
	super()
			

func _contains(tile_qr: Vector2i) -> bool:
	assert(_region_object != null)
	return _region_object.contains(tile_qr)

func load_region(region_object : RegionObject):
	assert(region_object != null, "Can't load nullptr as region")
	clear()
	
	_region_object = region_object
	
	print("type of region object:", type_string(typeof(region_object)))
	
	# dimensions are in (q,r,s) system with s omited
	# tilemap is in (x,y) system
	var qr_dimensions : Rect2i = region_object.get_dimensions()
	print("region dimensions: ", qr_dimensions)
	

	for q in range(qr_dimensions.position.x, qr_dimensions.end.x):
		for r in range(qr_dimensions.position.y, qr_dimensions.end.y):
			var qr_coords = Vector2i(q,r)
			if _contains(qr_coords):
				update_cell(qr_coords)
	# TODO: Connect to 'cell_updated' signal
			
				
func update_cell(qr_coords: Vector2i):
	var terrain = _region_object.get_cell_info(qr_coords)["terrain"]
	#print("Terrain of a cell qr=", Vector2i(q,r), " is \"", terrain, "\"")
	# convert to xy dimensions
	var xy_coords = QrsCoordsLibrary.qrs_to_xy(qr_coords)
	# fill cell
	if terrain_mapping.has(terrain):
		#print("setting terrail of tile map xy=", xy_coords, " to ", terrain_mapping[terrain])
		set_cell(0, xy_coords, 0, terrain_mapping[terrain].atlas_coords,0)
	else:
		# set cell to absent pink
		set_cell(0, xy_coords, 0, Vector2i(0,0), 0)
		
func get_region_object() -> RegionObject:
	return _region_object
	

