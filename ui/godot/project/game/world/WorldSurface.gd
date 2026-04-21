extends GameTileSurface

#signal enter_region(world_object: WorldObject, qr_position: Vector2i)
#signal region_clicked(world_object: WorldObject, qr_position: Vector2i)

var _plane: WorldPlane

var terrain_mapping : Dictionary = {}

func _ready() -> void:
	super()
	
func _contains(tile_qr: Vector2i) -> bool:
	if _plane == null:
		push_error("WorldSurface is running in uninitialized mode")
		return false

	assert(_plane != null)
	return _plane.plane_object.contains(tile_qr)


func get_plane() -> WorldPlane:
	return _plane

func load_plane(plane : WorldPlane) -> void:
	assert(plane != null, "Can't load nullptr as plane")
	clear()
	
	_plane = plane
		
	# dimensions are in (q,r,s) system with s omited
	# tilemap is in (x,y) system
	var qr_dimensions : Rect2i = _plane.get_qr_dimensions()
	debug_control.add_text("world dimensions: %s"  % qr_dimensions)
	
	var update_cell_lambda := func(region_q: int, region_r: int, region: RegionObject)-> void:
		update_cell(Vector2i(region_q, region_r), region)
		
	plane.plane_object.foreach_surface(update_cell_lambda)
	
	# Now, we can access get_used_rect
	$substrate.texture = plane.get_substrate()
	
	var used_rect : Rect2i = $biomes.get_used_rect() # this one is regions and in xy coords.
	var tile_size : Vector2i = $biomes.tile_set.tile_size
	var used_space := Vector2( used_rect.size.x * tile_size.x, used_rect.size.y * tile_size.y)
	var used_origin : Vector2 = -0.5 * Vector2(tile_size)
	
	debug_control.add_text("Tilemap rect: %s" % Rect2(used_origin, used_space))
	$substrate.position = used_origin
	$substrate.scale = used_space / plane.get_substrate().get_size()
	
	debug_control.add_text("Tilemap scale: %s" % $substrate.scale)

func update_cell(qr_coords: Vector2i, region: RegionObject) -> void:
	# convert to xy dimensions
	var xy_coords := axial_to_map(qr_coords)
	
	$biomes.set_cell(xy_coords, 0, Vector2i.ZERO, 1)
	pass
	
func _get_select_source_id():
	return 5
