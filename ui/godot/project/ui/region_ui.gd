extends Control

signal close_requested()
## Mmm, how was I going to use this signal?
signal region_cell_selected(region_object: RegionObject, qr: Vector2i)


func on_region_cell_clicked_forward(region_object: RegionObject, qr: Vector2i):
	region_cell_selected.emit(region_object, qr)
	
func load_region(region_object: RegionObject):
	# get list of buildings
	var ruleset = CurrentGame.current_player_ruleset
	# TODO: Filter only those improvements that can be build in this region
	var available_buildings = ruleset.get_all_region_improvements()
	for bld in available_buildings:
		$InfoTabContainer/Buildings.add_building(bld.id)
	
func _on_close_button_pressed():
	print("Close region UI requested") # TODO: RM
	close_requested.emit()
