extends Node2D

signal exit_reqion_request()
signal cell_selected(cell: CellObject)
signal cell_deselected(cell: CellObject)

var _region_object: RegionObject



func _ready() -> void:
	pass

func set_event_bus(event_bus: UiEventBus) -> void:
	$RegionSurface.event_bus = event_bus
	
func load_region(region_object : RegionObject) -> void:
	assert(region_object != null, "Can't load null region")
	if _region_object != null and region_object.get_region_id() == _region_object.get_region_id():
		# same region, do nothing
		return
	_clear_old_region()
	
	_region_object = region_object
	
	_connect_region_object(region_object)
	
	# notify every object that requires it that new region is loaded
	get_tree().call_group("ui:region", "load_region", region_object)
	
	#$RegionSurface.load_region(region_object)
	#$CanvasLayer/RegionUI.load_region(region_object)
	
# TODO: Cleare world ui events handling
func on_ui_event(event: UiEventBus.UIEvent) -> void:
	if event is UiEventBus.UIMovementEvent:
		if event.prev_qr_coords != event.qr_coords:
			event.surface.clear_highlight(event.prev_qr_coords)
		event.surface.highlight(event.qr_coords, true)
		
	if event is UiEventBus.RegionUIActionEvent:
		if event.action_type == UiEventBus.ActionType.PRIMARY:
			if not event.surface.is_selected(event.qr_coords):
				event.surface.clear_all_select()
				event.surface.select(event.qr_coords, true)
				assert(event.surface.surface_type == GameTileSurface.SurfaceType.REGION_SURFACE)
				# TODO: Check that event region and our region match
				var region: RegionObject = event.surface.get_region()
				var cell := region.get_cell(event.qr_coords)
				cell_selected.emit(cell)
			event.accept()
			return

	# TODO: first, clear selection, and only then exit region
	# if event is UiEventBus.CancellationEvent:
	# 	clear selection
	#	event.accept()
	#	return

	if event is UiEventBus.CancellationEvent:
		exit_reqion_request.emit()
		event.accept()
		return

func set_visualization(vis_table: Dictionary) -> void:
	$RegionSurface.visualization_data = vis_table
	
func _clear_old_region()->void:
	if _region_object == null:
		# no old region, exit
		return
		
	# break connections
	_disconnect_region_object(_region_object)

	
func _on_region_ui_close_requested() -> void:
	print("emiting exit region request") # TODO: RM
	exit_reqion_request.emit()
	
func _connect_region_object(_region_object_: RegionObject) -> void:
	# This class doesn't require any connections
	pass
	
func _disconnect_region_object(_region_object_: RegionObject) -> void:
	# keep it in sync with method _connect_region_object
	pass
