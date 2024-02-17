extends Control

signal close_requested()
# internal signal to notify internal components that region was loaded
signal _region_loaded(region_object: RegionObject)
signal region_cell_selected(region_object: RegionObject, qr: Vector2i)

var _interaction
var _region : RegionObject

func on_region_cell_clicked_forward(region_object: RegionObject, qr: Vector2i):
	region_cell_selected.emit(region_object, qr)
	
func load_region(region_object: RegionObject):
	assert(region_object != null)
	# get list of buildings
	var ruleset = CurrentGame.get_current_player_ruleset()
	# TODO: Filter only those improvements that can be build in this region
	# TODO: Move to signal
	var available_buildings = ruleset.get_all_region_improvements()
	for bld in available_buildings:
		$InfoTabContainer/Buildings.add_building(bld.id)
		
	region_selected.emit(region_object)
	
func _on_close_button_pressed():
	print("Close region UI requested") # TODO: RM
	close_requested.emit()


func _on_info_button_toggled(toggled_on):
	$InfoTabContainer.visible = toggled_on


func _on_build_button_toggled(toggled_on):
	$ScrollContainer.visible = toggled_on

		


func _on_city_button_pressed() -> void:
	CurrentGame.get_current_player_civ().create_city()
	#var city_build_interaction = SelectAndBuildInteraction.new()
	#GameUiEventBus.set_main_interaction(city_build_interaction)



func _on_build_improvement(improvement_id: String) -> void:
	print("_on_build_improvement: ", improvement_id)
	if _interaction != null:
		_interaction.cancel()
	var new_build_interaction = SelectAndBuildInteraction.new()
	new_build_interaction.improvement_id = improvement_id
	_interaction = new_build_interaction
	GameUiEventBus.set_main_interaction(new_build_interaction)
