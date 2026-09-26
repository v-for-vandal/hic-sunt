extends InteractionInterface

var _last_highlight_surface: GameTileSurface


func _init() -> void:
	pass


func on_ui_event(event: UiEventBus.UIEvent) -> void:
	if event is UiEventBus.RegionUIActionEvent:
		if event.action_type == UiEventBus.ActionType.PRIMARY:
			# TODO: Check that we can build here
			var can_build := true

			# get current region
			var region: RegionObject = event.surface.get_region()
			create_city_and_finish(region, event.qr_coords)
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


func create_city_and_finish(region: RegionObject, qr_coords: Vector2i) -> void:
	# stop receiving other events
	CurrentGame.event_bus.remove_main_interaction(self)
	var cell := region.get_cell(qr_coords)
	
	var create_result := CurrentGame.game.get_current_player_civ().create_city(cell)
	if not create_result.is_ok():
		push_error("Failed to create city")

	# cleanup
	cleanup()


func cleanup() -> void:
	if _last_highlight_surface != null:
		_last_highlight_surface.clear_all_highlight()


func _to_string() -> String:
	return "SelectAndCreateCity#%s" % get_instance_id()
