extends Node2D


signal show_region_request(world_object: WorldObject,  qr_position: Vector2i)

func load_world(world_object : WorldObject):
	$WorldSurface.load_world(world_object)

func set_terrain_visualization(vis_table: Dictionary):
	$WorldSurface.terrain_mapping = vis_table


func _on_world_surface_enter_region(world_object: WorldObject, qr_position: Vector2i):
	print("emiting enter region request")
	show_region_request.emit(world_object, qr_position)

