extends GameTileSurface

signal cell_clicked(region_object: RegionObject, qr_position: Vector2i)

var _region_object : RegionObject
var _improvements_layer_id : int = 0

var visualization_data : Dictionary = {}



func _ready() -> void:
	super()
	for i in range(get_layers_count()):
		if get_layer_name(i) == "improvements":
			_improvements_layer_id = i
			break


func _contains(tile_qr: Vector2i) -> bool:
	assert(_region_object != null)
	return _region_object.contains(tile_qr)

func load_region(region_object : RegionObject) -> void:
	assert(region_object != null, "Can't load nullptr as region")
	clear()
	
	_region_object = region_object
	
	# dimensions are in (q,r,s) system with s omited
	# tilemap is in (x,y) system
	var qr_dimensions : Rect2i = region_object.get_dimensions()

	for q in range(qr_dimensions.position.x, qr_dimensions.end.x):
		for r in range(qr_dimensions.position.y, qr_dimensions.end.y):
			var qr_coords := Vector2i(q,r)
			if _contains(qr_coords):
				update_cell(qr_coords)
	# TODO: Connect to 'cell_updated' signal
			
				
func update_cell(qr_coords: Vector2i) -> void:
	var region_info : Dictionary = _region_object.get_cell_info(qr_coords)
	var terrain : String = region_info.terrain
	#print("Terrain of a cell qr=", Vector2i(q,r), " is \"", terrain, "\"")
	# convert to xy dimensions
	var xy_coords := QrsCoordsLibrary.qrs_to_xy(qr_coords)
	# fill cell
	if visualization_data.has(terrain):
		#print("setting terrail of tile map xy=", xy_coords, " to ", terrain_mapping[terrain])
		set_cell(0, xy_coords, visualization_data[terrain].source_id,
			visualization_data[terrain].atlas_coords,0)
	else:
		# set cell to absent pink
		set_cell(0, xy_coords, 0, Vector2i(0,0), 0)
		
	var improvement = region_info.improvement
	if !improvement.is_empty():
		var improvement_type = improvement.type
		if improvement_type in visualization_data:
			# put it into layer above
			set_cell(_improvements_layer_id, xy_coords, visualization_data[improvement_type].source_id,
				visualization_data[improvement_type].atlas_coords,0)
		else:
			# place pink object to indicate an error
			push_error("Undefined visualization data for %s" % [improvement_type])
			set_cell(_improvements_layer_id, xy_coords, 0, Vector2i.ZERO, 0)
		
func get_region_object() -> RegionObject:
	return _region_object
	
func on_region_changed(area: Rect2i) -> void:
	for q in range(area.position.x, area.end.x):
		for r in range(area.position.y, area.end.y):
			var qr_coords := Vector2i(q,r)
			if _contains(qr_coords):
				update_cell(qr_coords)
	
	

