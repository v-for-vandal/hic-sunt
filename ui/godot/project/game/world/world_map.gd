extends Node

var _world : World
var _world_object: WorldObject
var _current_plane: WorldPlane


signal show_region_request(plane: WorldPlane,  qr_position: Vector2i)

func _ready() -> void:
	pass

func load_world(world : World) -> void:
	_world = world
	var planes := world.get_planes()
	if planes.size() > 0:
		_current_plane = world.get_plane(planes[0])
		$WorldSurface.load_plane(_current_plane)
	else:
		push_error('Can not load world without any planes')

func set_terrain_visualization(vis_table: Dictionary) -> void:
	$WorldSurface.terrain_mapping = vis_table
	
func on_ui_event(event: UiEventBus.UIEvent) -> bool:
	var as_world_event := event as UiEventBus.WorldUIActionEvent
	if as_world_event != null:
		if as_world_event.action_type == UiEventBus.ActionType.PRIMARY:
			show_region_request.emit(_world.world_object, event.qr_coords)
			return true
			
	return false


#func _on_world_surface_enter_region(world_object: WorldObject, qr_position: Vector2i):
	#print("emiting enter region request")
	#show_region_request.emit(world_object, qr_position)



func _on_world_ui_next_turn_requested() -> void:
	CurrentGame.next_turn()
