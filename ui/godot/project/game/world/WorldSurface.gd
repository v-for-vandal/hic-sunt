extends GameTileSurface

#signal enter_region(world_object: WorldObject, qr_position: Vector2i)
#signal region_clicked(world_object: WorldObject, qr_position: Vector2i)

var _plane_object : PlaneObject

var terrain_mapping : Dictionary = {}

func _ready() -> void:
	super()
	
func _contains(tile_qr: Vector2i) -> bool:
	if _plane_object == null:
		push_error("WorldSurface is running in uninitialized mode")
		return false

	assert(_plane_object != null)
	return _plane_object.contains(tile_qr)


func get_plane() -> PlaneObject:
	return _plane_object

func load_plane(plane_object : PlaneObject) -> void:
	assert(plane_object != null, "Can't load nullptr as world")
	clear()
	
	_plane_object = plane_object
		
	# dimensions are in (q,r,s) system with s omited
	# tilemap is in (x,y) system
	var qr_dimensions : Rect2i = _plane_object.get_dimensions()
	print("world dimensions: ", qr_dimensions)
	
	var update_cell_lambda := func(region_q: int, region_r: int, region: RegionObject)-> void:
		update_cell(Vector2i(region_q, region_r), region)
		
	plane_object.foreach_surface(update_cell_lambda)

func update_cell(qr_coords: Vector2i, region: RegionObject) -> void:
	# convert to xy dimensions
	var xy_coords := axial_to_map(qr_coords)
	
	$biomes.set_cell(xy_coords, 0, Vector2i.ZERO, 1)
	pass
	
func _get_select_source_id():
	return 5
