extends RefCounted

class_name  SelectAndBuildInteraction

var improvement_id: String
var city

var _last_highlight_surface : GameTileSurface
var _last_highlight_qr : Vector2i
	
func on_ui_event(event: GameUiEventBus.UIEvent):
	if event is GameUiEventBus.RegionUIActionEvent:
		if event.action_type == GameUiEventBus.ActionType.PRIMARY:
			# TODO: Check that we can build here
			var can_build := true
			
			# get current region
			var region : RegionObject = event.surface.get_region_object()
			# add to build queue
			# TODO: Region may not have world_qr
			#city.add_to_build(improvement_id, region, event.qr_coords)
			print("Building ", improvement_id, " at ", region.get_region_id())
			build_and_finish()
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
	
func build_and_finish() -> void:
	# stop receiving other events
	GameUiEventBus.remove_main_interaction(self)
	# cleanup
	cleanup()
	
func cleanup() -> void:
	if _last_highlight_surface != null:
		_last_highlight_surface.clear_all_highlight()
	
func _to_string() -> String:
	return "SelectAndBuild#%s" % get_instance_id()
