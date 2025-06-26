extends TabContainer

var _current_region: RegionObject
var _current_cell : Vector2i

func set_region(region: RegionObject) -> void:
	if _current_region != null:
		if _current_region == region:
			return
		if _current_region.get_region_id() == region.get_region_id():
			return
			
	var data = region.get_info()
	
	var data_table : Array[Array] = []
	for key in data:
		data_table.append([key, data[key]])
		
	%CellInfo.set_data(data_table)
	_current_region = region
	
func set_cell(qr_coords: Vector2i) -> void:
	if _current_cell == qr_coords:
			return
	if _current_region == null:
		return
			
	var data = _current_region.get_cell_info(qr_coords)
	
	var data_table : Array[Array] = []
	for key in data:
		data_table.append([key, data[key]])
		
	%CellInfo.set_data(data_table)
	_current_cell = qr_coords
