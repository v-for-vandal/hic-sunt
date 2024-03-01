extends ItemList

signal building_selected(building_id: String);

func _init() -> void:
	clear()

func add_building(building_id: String) -> void:
	var idx := add_item(building_id)
	set_item_metadata(idx, building_id)

func load_region(region: RegionObject) -> void:
	var ruleset : RulesetObject = CurrentGame.get_current_player_ruleset()
	var available_buildings = ruleset.get_all_region_improvements()
	clear()
	for bld in available_buildings:
		add_building(bld.id)


func _on_item_activated(index: int) -> void:
	building_selected.emit(get_item_metadata(index))
