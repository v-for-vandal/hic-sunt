# Global state that manages current game and its logic

extends Node

var current_world : WorldObject
var current_player_ruleset: RulesetObject

func init_game(world_object: WorldObject, ruleset: RulesetObject):
	current_world = world_object
	current_player_ruleset = ruleset

