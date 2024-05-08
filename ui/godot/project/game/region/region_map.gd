extends Node2D

signal exit_reqion_request()

var _region_object: RegionObject

func _ready() -> void:
	pass

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
	
func on_ui_event(event : Variant) -> void:
	if event is GameUiEventBus.CancellationEvent:
		event.accept()
		exit_reqion_request.emit()
		return

func _connect_region_object(region_object: RegionObject) -> void:
	# This class doesn't require any connections
	pass
	
func _disconnect_region_object(region_object: RegionObject) -> void:
	# keep it in sync with method _connect_region_object
	pass
