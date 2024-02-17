extends Node2D

var _world_object: WorldObject


signal show_region_request(world_object: WorldObject,  qr_position: Vector2i)

func _ready() -> void:
	$WorldViewCamera.cell_size = $WorldSurface.tile_set.tile_size.x
	GameUiEventBus.add_consumer(self)

func load_world(world_object : WorldObject):
	_world_object = world_object
	$WorldSurface.load_world(world_object)

func set_terrain_visualization(vis_table: Dictionary):
	$WorldSurface.terrain_mapping = vis_table
	
func on_ui_event(event: GameUiEventBus.UIEvent) -> bool:
	var as_world_event = event as GameUiEventBus.WorldUIActionEvent
	if as_world_event != null:
		if as_world_event.action_type == GameUiEventBus.ActionType.PRIMARY:
			show_region_request.emit(_world_object, event.qr_coords)
			return true
			
	return false


#func _on_world_surface_enter_region(world_object: WorldObject, qr_position: Vector2i):
	#print("emiting enter region request")
	#show_region_request.emit(world_object, qr_position)



func _on_world_ui_next_turn_requested():
	CurrentGame.next_turn()
