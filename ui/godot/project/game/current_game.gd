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
	current_world = world_object
	current_player_ruleset = ruleset
	_current_player_civ = Civilisation.create_civilisation()
	
func next_turn():
	_current_player_civ.next_turn()
	
# TODO: move to ruleset?
func can_build(improvement_id: String) -> bool:
	return true


func get_new_id():
	# returns new unique id. This id is globaly unique for whole game
	var result = _next_id
	_next_id += 1
	return result
