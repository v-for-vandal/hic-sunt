extends VBoxContainer

signal city_selected(city_id: String)

@export
var show_vassals := true:
	set(value):
		show_vassals = value
		_update_section_visibility()

@export
var show_foreign := true:
	set(value):
		show_foreign = value
		_update_section_visibility()

var _city_list_item_scene := preload("res://ui/city/city_list_item.tscn")


func _ready() -> void:
	_update_section_visibility()
	refresh()


func refresh() -> void:
	_clear_section(%MyCitiesRows)
	_clear_section(%VassalCitiesRows)
	_clear_section(%ForeignCitiesRows)

	if CurrentGame.current_game == null:
		_update_empty_sections()
		return

	var current_civ: Civilisation = CurrentGame.current_game.get_current_player_civ()
	if current_civ != null:
		_add_cities_to_section(%MyCitiesRows, current_civ.get_cities())

	# TODO: Fill this section when vassal relationships are implemented.

	if show_foreign:
		_add_foreign_cities(current_civ)

	_update_empty_sections()


func add_city(city: City) -> void:
	if city == null:
		return
	_add_city_to_section(%MyCitiesRows, city)
	_update_empty_sections()


func _add_foreign_cities(current_civ: Civilisation) -> void:
	var world := CurrentGame.current_game.world
	if world == null:
		return

	for civ: Civilisation in world.get_civilisations():
		if current_civ != null and civ.id() == current_civ.id():
			continue
		_add_cities_to_section(%ForeignCitiesRows, civ.get_cities())


func _add_cities_to_section(section_rows: VBoxContainer, cities: Array[City]) -> void:
	for city: City in cities:
		_add_city_to_section(section_rows, city)


func _add_city_to_section(section_rows: VBoxContainer, city: City) -> void:
	var item := _city_list_item_scene.instantiate()
	section_rows.add_child(item)
	item.setup(city)
	item.city_selected.connect(_on_city_item_selected)


func _clear_section(section_rows: VBoxContainer) -> void:
	for child: Node in section_rows.get_children():
		section_rows.remove_child(child)
		child.queue_free()


func _update_section_visibility() -> void:
	if not is_node_ready():
		return

	%VassalCitiesSection.visible = show_vassals
	%ForeignCitiesSection.visible = show_foreign


func _update_empty_sections() -> void:
	_update_section_state(%MyCitiesSection, %MyCitiesRows, %MyCitiesEmptyLabel)
	_update_section_state(%VassalCitiesSection, %VassalCitiesRows, %VassalCitiesEmptyLabel)
	_update_section_state(%ForeignCitiesSection, %ForeignCitiesRows, %ForeignCitiesEmptyLabel)


func _update_section_state(section: Control, rows: VBoxContainer, empty_label: Label) -> void:
	var is_empty := rows.get_child_count() == 0
	empty_label.visible = is_empty
	section.modulate = Color(0.55, 0.55, 0.55, 1.0) if is_empty else Color.WHITE


func _on_city_item_selected(city_id: String) -> void:
	city_selected.emit(city_id)
