extends Node2D

signal exit_reqion_request()

var _region_object: RegionObject

func _ready() -> void:
	GlobalSignalsBus.region_changed.connect(_on_region_changed)

func load_region(region_object : RegionObject) -> void:
	_region_object = region_object
	$RegionSurface.load_region(region_object)
	$CanvasLayer/RegionUI.load_region(region_object)

func set_visualization(vis_table: Dictionary) -> void:
	$RegionSurface.visualization_data = vis_table
	
func _on_region_ui_close_requested() -> void:
	print("emiting exit region request") # TODO: RM
	exit_reqion_request.emit()
	
func on_ui_event(event : Variant) -> void:
	if event is GameUiEventBus.CancellationEvent:
		event.accept()
		exit_reqion_request.emit()
		return
		
func _on_region_changed(region_id: String, area: Rect2i) -> void:
	if region_id != _region_object.get_region_id():
		# Not currently loaded region, so do nothing
		return
		
	$RegionSurface.on_region_changed(area)
	$CanvasLayer/RegionUI.on_region_changed(area)
