# Global state that manages current game and its logic
extends Node

var current_game: Game
var event_bus : UiEventBus = UiEventBus.new()

func init_game(world: World, ruleset: RulesetObject) -> void:
	current_game = Game.new(world, ruleset)
	event_bus = UiEventBus.new()

func next_turn() -> void:
	if current_game == null:
		push_error("Attempt to execute next_turn on non-created game")
		return
	return current_game.next_turn()

func save_game(save_location: DirAccess) -> Error:
	if current_game == null:
		return ERR_DOES_NOT_EXIST
		
	return current_game.save_game(save_location)
	


func load_game(save_location: DirAccess, ruleset: RulesetObject) -> Error:
	event_bus = UiEventBus.new()
	current_game = Game.load_game(save_location, ruleset)
	if current_game == null:
		return ERR_BUG

	return OK
