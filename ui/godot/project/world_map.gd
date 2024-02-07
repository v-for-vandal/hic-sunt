extends Node2D


signal show_region_request(world_object: WorldObject,  qr_position: Vector2i)

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

func load_world(world_object : WorldObject):
	$WorldSurface.load_world(world_object)
	


func _on_world_surface_enter_region(world_object: WorldObject, qr_position: Vector2i):
	print("emiting enter region request")
	show_region_request.emit(world_object, qr_position)

