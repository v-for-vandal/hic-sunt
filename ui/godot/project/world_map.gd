extends Node2D



# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

func load_world(world_object : WorldObject):
	$WorldSurface.load_world(world_object)

