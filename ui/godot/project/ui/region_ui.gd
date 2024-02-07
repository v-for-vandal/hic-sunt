extends Control

signal close_requested()
## Mmm, how was I going to use this signal?
signal region_cell_selected(region_object: RegionObject, qr: Vector2i)


func on_region_cell_clicked_forward(region_object: RegionObject, qr: Vector2i):
	region_cell_selected.emit(region_object, qr)
	
func _on_close_button_pressed():
	print("Close region UI requested") # TODO: RM
	close_requested.emit()
