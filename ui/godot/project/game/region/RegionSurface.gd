extends GameTileSurface

signal cell_clicked(region_object: RegionObject, qr_position: Vector2i)

var _region_object : RegionObject

# Those constants are set up in _ready()
var _BIOME_LAYER : TileMapLayer = null
var _IMPROVEMENT_LAYER : TileMapLayer  = null

# TODO: Deprecated, remove it
var visualization_data : Dictionary = {}



func _ready() -> void:
	super()

	for child in get_children():
		if child is not TileMapLayer:
			continue
		if child.name == "improvements":
			_IMPROVEMENT_LAYER = child
		if child.name == "biomes":
			_BIOME_LAYER = child
			
	#assert(_BIOME_LAYER != null, "Faild to find TileMap layer for biomes")
	assert(_IMPROVEMENT_LAYER != null, "Faild to find TileMap layer for improvements")
	print("Region surface node is ready")


func _contains(tile_qr: Vector2i) -> bool:
	if _region_object == null:
		return false

	return _region_object.contains(tile_qr)

func load_region(region_object : RegionObject) -> void:
	assert(region_object != null, "Can't load nullptr as region")
	
	clear()
	
	_connect_region_object(region_object)
	_region_object = region_object

	
	
	var calc_xy_bounding_rect := func(region_q: int, region_r: int, region: RegionObject) -> void:
		var offset_coords := axial_to_map(Vector2i(region_q, region_r))
		
	var update_cell_lambda := func(cell_q: int, cell_r: int)-> void:
		update_cell(Vector2i(cell_q, cell_r))
		
	region_object.foreach(update_cell_lambda)


func clear()->void:
	super.clear()
	
	if _region_object == null:
		# no old region, exit
		return
		
	# break connections
	_disconnect_region_object(_region_object)
	
func update_cell(qr_coords: Vector2i) -> void:
	var region_info : Dictionary = _region_object.get_cell_info(qr_coords)
	

		
	# convert to xy dimensions
	var xy_coords := axial_to_map(qr_coords)
	
	$biomes.set_cell(xy_coords, 0, Vector2i.ZERO, 1)

	var improvement = region_info.improvement
	if !improvement.is_empty():
		var improvement_type = improvement.type
		if improvement_type in visualization_data:
			# put it into layer above
			_IMPROVEMENT_LAYER.set_cell(xy_coords, visualization_data[improvement_type].source_id,
				visualization_data[improvement_type].atlas_coords,0)
		else:
			# place pink object to indicate an error
			push_error("Undefined visualization data for %s" % [improvement_type])
			_IMPROVEMENT_LAYER.set_cell(xy_coords, 0, Vector2i.ZERO, 0)
	else:
		# no improvement
		_IMPROVEMENT_LAYER.erase_cell(xy_coords)
		
func get_region() -> RegionObject:
	return _region_object
	
func _on_region_changed(area: Rect2i, flags: int) -> void:
	for q in range(area.position.x, area.end.x):
		for r in range(area.position.y, area.end.y):
			var qr_coords := Vector2i(q,r)
			if _contains(qr_coords):
				update_cell(qr_coords)
	
func _connect_region_object(region_object: RegionObject) -> void:
	region_object.region_changed.connect(_on_region_changed)
	
func _disconnect_region_object(region_object: RegionObject) -> void:
	region_object.region_changed.disconnect(_on_region_changed)
	
func _get_select_source_id():
	return 5
