extends Node

signal region_changed(region_id: String, area_changed: Rect2i)

func emit_region_cell_changed(region_id: String, qr_coords: Vector2i) -> void:
	region_changed.emit(region_id, Rect2i(qr_coords, Vector2i.ONE))
	
func emit_region_area_changed(region_id: String, qr_area: Rect2i) -> void:
	region_changed.emit(region_id, qr_area)
