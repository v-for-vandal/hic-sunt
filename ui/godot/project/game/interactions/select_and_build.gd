extends RefCounted

class_name  SelectAndBuildInteraction

var improvement_id: String
var city
	
func on_ui_event(event: GameUiEventBus.UIEvent):
	if event is GameUiEventBus.RegionUIActionEvent:
		if event.action_type == GameUiEventBus.ActionType.PRIMARY:
			# TODO: Check that we can build here
			var can_build := true
			
			# get current region
			var region : RegionObject = event.surface.get_region_object()
			# add to build queue
			# TODO: Region may not have world_qr
			city.add_to_build(improvement_id, region, event.qr_coords)
			cleanup()
			event.accept()
			
	elif event is GameUiEventBus.RegionUIMovementEvent:
		event.surface.clear_highlight(event.prev_qr_coords)
		event.surface.highlight(event.qr_coords)
		event.accept()
		
		
func start():
		
		
func cancel():
