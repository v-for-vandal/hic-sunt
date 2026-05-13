extends Node

var _world : World
var _current_plane: WorldPlane


signal show_region_request(region: RegionObject)

signal region_selected(region: RegionObject)
signal region_deselected(region: RegionObject)


func _ready() -> void:
	pass
	
func set_event_bus(event_bus: UiEventBus) -> void:
	$WorldSurface.event_bus = event_bus

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

func on_ui_event(event: UiEventBus.UIEvent) -> void:
	if event is UiEventBus.WorldUIMovementEvent:
		if event.prev_qr_coords != event.qr_coords:
			event.surface.clear_highlight(event.prev_qr_coords)
		event.surface.highlight(event.qr_coords, true)
		event.accept()
		return
		
	if event is UiEventBus.WorldUIActionEvent:
		if event.action_type == UiEventBus.ActionType.PRIMARY:
			if event.surface.is_selected(event.qr_coords):
				var plane: WorldPlane = event.surface.get_plane()
				var region := plane.plane_object.get_region(event.qr_coords)
				show_region_request.emit(region)
			else:
				event.surface.clear_all_select()
				event.surface.select(event.qr_coords, true)
				var plane: WorldPlane = event.surface.get_plane()
				var region := plane.plane_object.get_region(event.qr_coords)
				region_selected.emit(region)
			event.accept()
			return

	if event is UiEventBus.CancellationEvent:
		var selected : Array[Vector2i] = event.surface.get_selected()
		if selected.size() > 0:
			var plane : WorldPlane = event.surface.get_plane()
			if plane:
				for coords in selected:
					assert(event.surface.contains(coords))
					region_deselected.emit(plane.plane_object.get_region(coords))
			event.surface.clear_all_selected()
			event.accept()
			return



#func _on_world_surface_enter_region(world_object: WorldObject, qr_position: Vector2i):
	#print("emiting enter region request")
	#show_region_request.emit(world_object, qr_position)



func _on_world_ui_next_turn_requested() -> void:
	CurrentGame.next_turn()
