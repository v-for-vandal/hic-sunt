# Global state that manages current game and its logic
extends Node

class_name Game

signal new_turn_started(turn_number: int)

var _world: World
var _ruleset: RulesetObject
var _session : SessionObject
var _debug_control : DebugTree.ControlInterface

var world: World:
	get:
		return _world

var ruleset: RulesetObject:
	get:
		return _ruleset

var debug_control: DebugTree.ControlInterface:
	get:
		return _debug_control
		
var session: SessionObject:
	get:
		return _session

# TODO: Move to World
var _current_player_civ: Civilisation
# var _current_turn := 1

var _next_id := 0




func _init(world_: World, ruleset_: RulesetObject) -> void:
	_session = SessionObject.new()
	_session.set_world(world_.world_object)
	_session.set_ruleset(ruleset_)
	_world = world_
	_ruleset = ruleset_
	assert(_world)
	assert(_ruleset)
	_current_player_civ = Civilisation.create_civilisation(get_new_id())
# TODO: we must do something with current_turn and next_id

func _ready() -> void:
	_debug_control = DebugRoot.get_debug_control().add_random_group("Game-")
	# add all ruleset variables
	var variables_d_c := _debug_control.add_text_node("variables", "")
	var var_defines : Dictionary = _ruleset.get_variable_definitions()
	for varname: StringName in var_defines:
		var vardata : Dictionary = var_defines[varname]
		var var_type : StringName = vardata[&"type"];
		variables_d_c.add_text("%-80s| %-10s" % [varname, var_type])

func replace_ruleset(ruleset_: RulesetObject) -> void:
	_session.set_ruleset(ruleset_)

func get_current_player_civ() -> Civilisation:
	return _current_player_civ


func get_current_world() -> World:
	return _world


func get_current_turn() -> int:
	return _session.get_current_turn()



# TODO: REname it as end_turn
func next_turn() -> void:
	_debug_control.add_text("Advancing from %d to %d" % [get_current_turn(), get_current_turn() + 1])
	_session.advance_next_turn()
	_current_player_civ.next_turn()
	new_turn_started.emit(get_current_turn())


# TODO: move to ruleset?
func can_build(_improvement_id: String) -> bool:
	return true


func get_new_id() -> int:
	# returns new unique id. This id is globaly unique for whole game
	var result: int = _next_id
	_next_id += 1
	return result


static func _game_file_path(save_location: DirAccess) -> String:
	return save_location.get_current_dir() + '/' + 'game.json'

static func _world_file_path(save_location: DirAccess) -> String:
	return save_location.get_current_dir() + '/' + 'world.data'

func save_game(save_location: DirAccess) -> Error:
	if _world == null:
		return ERR_DOES_NOT_EXIST
		

	var world_file_path := _world_file_path(save_location)
	# TODO: Restore
	var status : Error = _world.save(ProjectSettings.globalize_path(world_file_path))
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


static func load_game(save_location: DirAccess, new_ruleset: RulesetObject) -> Game:
	# Create empty world
	var new_world: World = World.new()
	assert(new_world != null)
	var world_file_path := _world_file_path(save_location)
	var status: Error = new_world.load(ProjectSettings.globalize_path(world_file_path))
	
	if status != OK:
		return null
		
	var game := Game.new(new_world, new_ruleset)

	var game_data_path := _game_file_path(save_location)
	var game_data_file := FileAccess.open(game_data_path, FileAccess.READ)
	if game_data_file == null:
		push_error("Can't open file %s for reading: %s" % [game_data_path, FileAccess.get_open_error()])
		return null

	var json := JSON.new()
	if json.parse(game_data_file.get_as_text()) != OK:
		push_error("Can't parse file %s as json" % [game_data_file])
		return null

	var civ := Civilisation.new()
	civ.parse_from_variant(json.data)

	# TODO: WE have to actually verify that data between binary file and json
	# file match each other. Otherwise, one could add new city into json file
	# without it being marked in world files

	game._current_player_civ = civ

	return game
