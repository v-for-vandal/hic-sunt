extends Node2D

var terrain_mapping : Dictionary = {
	"coast" : Vector2i(0,0),
	"plains" : Vector2i(1,0),
	"sand" : Vector2i(2,0),
	"ocean" : Vector2i(3,0),
	"snow" : Vector2i(4,0),
}

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

func _qrs_to_xy(q:int, r:int)->Vector2i :
	var col = q  + (r - (r&1)) / 2
	var row = r
	return Vector2i(col, row)

func load_world(world_object : WorldObject):
	assert(world_object != null, "Can't load nullptr as world")
	$WorldSurface.clear()
	
	print("type of world object:", type_string(typeof(world_object)))
	
	# dimensions are in (q,r,s) system with s omited
	# tilemap is in (x,y) system
	var qr_dimensions = world_object.get_dimensions()
	
	var q_len = qr_dimensions.x
	var r_len = qr_dimensions.y

	for q in q_len:
		for r in r_len:
			var terrain = world_object.get_cell_terrain(Vector2i(q,r))
			# convert to xy dimensions
			var xy_coords = _qrs_to_xy(q,r)
			# fill cell
			if terrain_mapping.has(terrain):
				$WorldSurface.set_cell(0, xy_coords, 0, terrain_mapping[terrain],0)
