extends InteractionInterface

var _improvement_id: String

var _last_highlight_surface: GameTileSurface


func _init(improvement_id: String) -> void:
	self._improvement_id = improvement_id


func on_ui_event(event: UiEventBus.UIEvent) -> void:
	if event is UiEventBus.RegionUIActionEvent:
		if event.action_type == UiEventBus.ActionType.PRIMARY:
			# TODO: Check that we can build here

			# get current region
			var region: RegionObject = event.surface.get_region()
			build_and_finish(region, event.qr_coords)
			event.accept()
			return

	elif event is UiEventBus.RegionUIMovementEvent:
		if event.prev_qr_coords != event.qr_coords:
			event.surface.clear_highlight(event.prev_qr_coords)
		event.surface.highlight(event.qr_coords, true)
		_last_highlight_surface = event.surface
		event.accept()
		return

	elif event is UiEventBus.CancellationEvent:
		print("Cancelling building")
		cancel()
		event.accept()


func cancel() -> void:
	# stop receiving other events
	CurrentGame.event_bus.remove_main_interaction(self)
	# cleanup
	cleanup()


func build_and_finish(region: RegionObject, qr_coords: Vector2i) -> void:
	# TODO: Perhaps we should not store this logic in interaction and instead
	# should move this code to civilization.gd
	print("Building ", _improvement_id, " at ", region.get_region_id())

	# stop receiving other events
	CurrentGame.event_bus.remove_main_interaction(self)
	var cell := region.get_cell(qr_coords)
	
	# TODO: Detect civilization id
	var civ_id := WorldConstants.UNOWNED_CIV
	var success : bool = CurrentGame.current_game.session.add_improvement(cell, civ_id, _improvement_id)
	if not success:
		push_error("Failed to build improvement")

	# cleanup
	cleanup()


func cleanup() -> void:
	if _last_highlight_surface != null:
		_last_highlight_surface.clear_all_highlight()


func _to_string() -> String:
	return "SelectAndBuild#%s" % get_instance_id()
