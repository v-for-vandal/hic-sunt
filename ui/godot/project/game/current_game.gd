# Global state that manages current game and its logic

extends Node

var current_world : WorldObject
# TODO: Make ruleset one for all players
var current_player_ruleset: RulesetObject
var _current_player_civ: Civilisation

var _next_id := 0

# TODO: Get rid of this method in favor of get_ruleset
func get_current_player_ruleset() -> RulesetObject:
	return current_player_ruleset
	
func get_ruleset() -> RulesetObject:
	return current_player_ruleset
	
func get_current_player_civ() -> Civilisation:
	return _current_player_civ
	
func get_atlas_visualization() -> Dictionary:
	# TODO:
	# 1. cache data
	# 2. move to some separate class
	return get_ruleset().get_atlas_render()

func init_game(world_object: WorldObject, ruleset: RulesetObject):
	print("Initializing game") # TODO: RM
	current_world = world_object
	current_player_ruleset = ruleset
	_current_player_civ = Civilisation.create_civilisation()
	
func next_turn() -> void:
	_current_player_civ.next_turn()
	
# TODO: move to ruleset?
func can_build(improvement_id: String) -> bool:
	return true


func get_new_id() -> int:
	# returns new unique id. This id is globaly unique for whole game
	var result : int = _next_id
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
	# TODO: save game data
	return OK
	
func load_game(save_location: DirAccess, ruleset: RulesetObject) -> Error:
	# Create empty world
	# TODO: Have some method to create empty world
	var world : WorldObject = CentralSystem.create_world(Vector2i(1, 1), 1, ruleset)
	assert(world != null)
	var world_file_path := _world_file_path(save_location)
	var status : Error = world.load(ProjectSettings.globalize_path(world_file_path))
	#var status := OK
	if status != OK:
		push_error("Can't load world from %s" % [_world_file_path(save_location)])
		return status
		
	init_game(world, ruleset)
	
	return OK
	
	
