# Global state that manages current game and its logic
extends Node

signal new_turn_started(turn_number: int)

var current_world: WorldObject
# TODO: Make ruleset one for all players
var current_player_ruleset: RulesetObject
var _current_player_civ: Civilisation
var _current_turn := 1

var _next_id := 0


# TODO: Get rid of this method in favor of get_ruleset
func get_current_player_ruleset() -> RulesetObject:
	return current_player_ruleset


func get_ruleset() -> RulesetObject:
	return current_player_ruleset


func get_current_player_civ() -> Civilisation:
	return _current_player_civ


func get_current_world() -> WorldObject:
	return current_world


func get_current_turn() -> int:
	return _current_turn


func get_atlas_visualization() -> Dictionary:
	# TODO:
	# 1. cache data
	# 2. move to some separate class
	return get_ruleset().get_atlas_render()


func init_game(world_object: WorldObject, ruleset: RulesetObject) -> void:
	print("Initializing game") # TODO: RM
	current_world = world_object
	current_player_ruleset = ruleset
	_current_player_civ = Civilisation.create_civilisation()
# TODO: we must do something with current_turn and next_id


# TODO: REname it as end_turn
func next_turn() -> void:
	_current_player_civ.next_turn()
	_current_turn += 1
	new_turn_started.emit(_current_turn)


# TODO: move to ruleset?
func can_build(_improvement_id: String) -> bool:
	return true


func get_new_id() -> int:
	# returns new unique id. This id is globaly unique for whole game
	var result: int = _next_id
	_next_id += 1
	return result


func _world_file_path(save_location: DirAccess) -> String:
	return save_location.get_current_dir() + '/' + 'world.data'


func _game_file_path(save_location: DirAccess) -> String:
	return save_location.get_current_dir() + '/' + 'game.json'


func save_game(save_location: DirAccess) -> Error:
	if current_world == null:
		return ERR_DOES_NOT_EXIST

	var world_file_path := _world_file_path(save_location)
	# TODO: Restore
	var status := current_world.save(ProjectSettings.globalize_path(world_file_path))
	if status != OK:
		return status

	var game_data_path := _game_file_path(save_location)
	var game_data_file := FileAccess.open(game_data_path, FileAccess.WRITE)
	if game_data_file == null:
		push_error("Can't open file %s for writing" % [game_data_path])
		return FileAccess.get_open_error()

	var serialize_data := _current_player_civ.serialize_to_variant()
	game_data_file.store_string(JSON.stringify(serialize_data, "\t"))

	return OK


func load_game(save_location: DirAccess, ruleset: RulesetObject) -> Error:
	current_player_ruleset = ruleset
	# Create empty world
	# TODO: Have some method to create empty world
	var world: WorldObject = WorldObject.new()
	assert(world != null)
	var world_file_path := _world_file_path(save_location)
	var status: Error = world.load(ProjectSettings.globalize_path(world_file_path))
	current_world = world

	#var status := OK
	if status != OK:
		push_error("Can't load world from %s" % [_world_file_path(save_location)])
		return status

	var game_data_path := _game_file_path(save_location)
	var game_data_file := FileAccess.open(game_data_path, FileAccess.READ)
	if game_data_file == null:
		push_error("Can't open file %s for reading" % [game_data_path])
		return FileAccess.get_open_error()

	var json := JSON.new()
	if json.parse(game_data_file.get_as_text()) != OK:
		push_error("Can't parse file %s as json" % [game_data_file])
		return ERR_FILE_CORRUPT

	var civ := Civilisation.new()
	civ.parse_from_variant(json.data)

	# TODO: WE have to actually verify that data between binary file and json
	# file match each other. Otherwise, one could add new city into json file
	# without it being marked in world files

	_current_player_civ = civ

	return OK

	# TODO: This should be in ruleset object, but for that we should
	# create a gdscript wrapper around gdextension/c++ class
	#func get
