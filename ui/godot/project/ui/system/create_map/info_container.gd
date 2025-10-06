extends TabContainer

var _current_region: RegionObject
var _current_cell : Vector2i

## region_qr_coords could be null if region is off-grid
func set_region(region: RegionObject, region_qr_coords : Variant) -> void:
	if _current_region != null:
		if _current_region == region:
			return
		if _current_region.get_region_id() == region.get_region_id():
			return
			
	var data = region.get_info()
	
	var data_table : Array[Array] = []
	data_table.append([&"region_id", region.get_id()])
	if region_qr_coords != null:
		data_table.append([&"qrs", QrsCoordsLibrary.qr_to_qrs(region_qr_coords as Vector2i)])
	for key in data:
		data_table.append([key, data[key]])
	data_table.append([&"height", region.get_scope().get_numeric_value(Modifiers.GEOGRAPHY_HEIGHT)])
	data_table.append([&"temperature", region.get_scope().get_numeric_value(Modifiers.ECOSYSTEM_TEMPERATURE)])
	data_table.append([&"precipitation", region.get_scope().get_numeric_value(Modifiers.ECOSYSTEM_PRECIPITATION)])

	%CellInfo.set_data(data_table)
	_current_region = region
	
func set_cell(qr_coords: Vector2i) -> void:
	if _current_cell == qr_coords:
			return
	if _current_region == null:
		return
	
	var cell = _current_region.get_cell(qr_coords)
	if not cell:
		return
	var data = _current_region.get_cell_info(qr_coords)
	
	var data_table : Array[Array] = []
	data_table.append([&"region_id", _current_region.get_id()])
	data_table.append([&"qrs", QrsCoordsLibrary.qr_to_qrs(qr_coords)])
	data_table.append([&"height", cell.get_scope().get_numeric_value(Modifiers.GEOGRAPHY_HEIGHT)])
	data_table.append([&"temperature", cell.get_scope().get_numeric_value(Modifiers.ECOSYSTEM_TEMPERATURE)])
	data_table.append([&"precipitation", cell.get_scope().get_numeric_value(Modifiers.ECOSYSTEM_PRECIPITATION)])

	for key in data:
		data_table.append([key, data[key]])
		
	%CellInfo.set_data(data_table)
	_current_cell = qr_coords
