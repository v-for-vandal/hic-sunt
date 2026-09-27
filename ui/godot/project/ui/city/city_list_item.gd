extends PanelContainer

signal city_selected(city_id: String)

var _city_id := ""


func setup(city: City) -> void:
	assert(city != null)
	if city == null:
		return

	_city_id = city.get_id()
	%CityNameLabel.text = _city_id
	%RegionLabel.text = "Region: %s" % city.get_primary_region_id()
	tooltip_text = "City %s\nPrimary region: %s" % [_city_id, city.get_primary_region_id()]


func get_city_id() -> String:
	return _city_id


func _gui_input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		var mouse_event := event as InputEventMouseButton
		if mouse_event.button_index == MOUSE_BUTTON_LEFT and mouse_event.pressed:
			city_selected.emit(_city_id)
			accept_event()
