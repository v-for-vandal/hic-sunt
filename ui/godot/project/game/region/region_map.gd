extends Node2D

signal exit_reqion_request()


func load_region(region_object : RegionObject):
	$RegionSurface.load_region(region_object)
	$CanvasLayer/RegionUI.load_region(region_object)

func set_terrain_visualization(vis_table: Dictionary):
	$RegionSurface.terrain_mapping = vis_table
	
func _on_region_ui_close_requested():
	print("emiting exit region request") # TODO: RM
	exit_reqion_request.emit()
	
func on_ui_event(event) -> void:
	if event is GameUiEventBus.CancellationEvent:
		event.accept()
		exit_reqion_request.emit()
		return
