extends RefCounted

var _improvement_id: String

var _last_highlight_surface : GameTileSurface

func _init(improvement_id : String) -> void:
	self._improvement_id = improvement_id

func on_ui_event(event: GameUiEventBus.UIEvent) -> void:
	if event is GameUiEventBus.RegionUIActionEvent:
		if event.action_type == GameUiEventBus.ActionType.PRIMARY:
			# TODO: Check that we can build here
			var can_build := true
			
			# get current region
			var region : RegionObject = event.surface.get_region_object()
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
	print("Building ", _improvement_id, " at ", region.get_region_id())
	
	# stop receiving other events
	GameUiEventBus.remove_main_interaction(self)
	# send city command to build improvement
	var city_id : String = region.get_city_id()
	if city_id.is_empty():
		# TODO: Actually, we should be able to build improvement in region
		# without city
		push_error("Dev mode: warning: building improvement without any city")

	region.set_improvement(qr_coords, _improvement_id)
		
	# cleanup
	cleanup()
	
func cleanup() -> void:
	if _last_highlight_surface != null:
		_last_highlight_surface.clear_all_highlight()
	
func _to_string() -> String:
	return "SelectAndBuild#%s" % get_instance_id()
