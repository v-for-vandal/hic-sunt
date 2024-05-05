extends RefCounted

class_name  SelectAndBuildInteraction

var improvement_id: String

var _last_highlight_surface : GameTileSurface


func on_ui_event(event: GameUiEventBus.UIEvent) -> void:
	if event is GameUiEventBus.RegionUIActionEvent:
		if event.action_type == GameUiEventBus.ActionType.PRIMARY:
			# TODO: Check that we can build here
			var can_build := true
			
			# get current region
			var region : RegionObject = event.surface.get_region_object()
			# add to build queue
			# TODO: Region may not have world_qr
			#city.add_to_build(improvement_id, region, event.qr_coords)
			build_and_finish(region, event.qr_coords)
			event.accept()
			return
			
	elif event is GameUiEventBus.RegionUIMovementEvent:
		if event.prev_qr_coords != event.qr_coords:
			event.surface.clear_highlight(event.prev_qr_coords)
		event.surface.highlight(event.qr_coords, true)
		_last_highlight_surface = event.surface
		event.accept()
		return
		
	elif event is GameUiEventBus.CancellationEvent:
		print("Cancelling building")
		cancel()
		event.accept()
		
		

func cancel() -> void:
	# stop receiving other events
	GameUiEventBus.remove_main_interaction(self)
	# cleanup
	cleanup()
	
func build_and_finish(region: RegionObject, qr_coords: Vector2i) -> void:
	# TODO: Perhaps we should not store this logic in interaction and instead
	# should move this code to civilization.gd
	print("Building ", improvement_id, " at ", region.get_region_id())
	
	# stop receiving other events
	GameUiEventBus.remove_main_interaction(self)
	# send city command to build improvement
	var city_id : String = region.get_city_id()
	if city_id.is_empty():
		# TODO: Actually, we should be able to build improvement in region
		# without city
		push_error("Incorrect attempt to build improvement without any city")
	else:
		var city : City = CurrentGame.get_current_player_civ().find_city_by_id(city_id)
		if city == null:
			push_error("Can't find specified city: ", city_id)
		else:
			var construction_project = ConstructionProject.create_construction_project(
				improvement_id, region, qr_coords)
			if construction_project == null:
				push_error("Failed to create construction project")
			else:
				city.add_project(construction_project)
		
	# cleanup
	cleanup()
	
func cleanup() -> void:
	if _last_highlight_surface != null:
		_last_highlight_surface.clear_all_highlight()
	
func _to_string() -> String:
	return "SelectAndBuild#%s" % get_instance_id()
