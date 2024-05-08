extends Control

signal close_requested()
signal region_cell_selected(region_object: RegionObject, qr: Vector2i)

var _interaction : Variant
var _region : RegionObject

func _init() -> void:
	pass
	
func _ready() -> void:
	pass

func on_region_cell_clicked_forward(region_object: RegionObject, qr: Vector2i) -> void:
	region_cell_selected.emit(region_object, qr)
	
func load_region(region_object: RegionObject) -> void:
	assert(region_object != null)
	_region = region_object
	_update_region()
	
func _update_region() -> void:
	if _region == null:
		return
	# get list of buildings
	#var ruleset := CurrentGame.get_current_player_ruleset()
	# TODO: Filter only those improvements that can be build in this region
	# TODO: Move to signal
		
	var city_id_opt := _region.get_city_id()
	print("Loading region ", _region.get_region_id(), " city id: ", city_id_opt)
	if city_id_opt.is_empty():
		$CityNameLabel.visible = false
	else:
		print("Got city: ", city_id_opt)
		$CityNameLabel.visible = true
		$CityNameLabel.text = city_id_opt
		
	# The call load_region is performed via call_group. All nodes
	# that require such call will receive them.
	#$InfoTabContainer/Buildings.load_region(_region)
	#$InfoTabContainer/Resources.load_region(_region)
	#$InfoTabContainer/Projects.load_region(_region)
	#$InfoTabContainer/Jobs.load_region(_region)
	#$ScrollContainer/VBoxContainer/BuildingList.load_region(_region)
	
func _on_close_button_pressed() -> void:
	print("Close region UI requested") # TODO: RM
	close_requested.emit()


func _on_info_button_toggled(toggled_on : bool) -> void:
	$InfoTabContainer.visible = toggled_on


func _on_build_button_toggled(toggled_on : bool) -> void:
	$ScrollContainer.visible = toggled_on

		


func _on_city_button_pressed() -> void:
	if CurrentGame.get_current_player_civ().can_create_city(_region.get_region_id()):
		CurrentGame.get_current_player_civ().create_city(_region.get_region_id())
	#var city_build_interaction = SelectAndBuildInteraction.new()
	#GameUiEventBus.set_main_interaction(city_build_interaction)



func _on_build_improvement(improvement_id: String) -> void:
	if _interaction != null:
		_interaction.cancel()
	var new_build_interaction := SelectAndBuildInteraction.new()
	new_build_interaction.improvement_id = improvement_id
	_interaction = new_build_interaction
	GameUiEventBus.set_main_interaction(new_build_interaction)
	
# This function is called by parent class when something inside region has changed
func _on_region_changed(_area: Rect2i, flags: int) -> void:
	if _region == null:
		return
		
	_update_region()
