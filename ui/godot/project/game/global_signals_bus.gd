extends Node

# area_changed - cells that have been changed. Parameter area_changed 
# is Rect2i() if change is not specific for the cell, for example new city
signal region_changed(region_id: String, area_changed: Rect2i)

# emited when city changed. This signal is often, but not always, is accompanied
# by region_changed signal
signal city_changed(city_id: String)

func emit_region_cell_changed(region_id: String, qr_coords: Vector2i) -> void:
	#region_changed.emit(region_id, Rect2i(qr_coords, Vector2i.ONE))
	emit_region_area_changed(region_id, Rect2i(qr_coords, Vector2i.ONE))
	
func emit_region_area_changed(region_id: String, qr_area: Rect2i) -> void:
	print("emiting region changed for ", region_id)
	print("connections: ", region_changed.get_connections())
	region_changed.emit(region_id, qr_area)
	
func emit_region_changed(region_id: String) -> void:
	#region_changed.emit(region_id, Rect2i())
	emit_region_area_changed(region_id, Rect2i())
