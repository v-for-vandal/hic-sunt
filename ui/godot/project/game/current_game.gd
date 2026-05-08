# Global state that manages current game and its logic
extends Node

var current_game: Game
var event_bus : UiEventBus = UiEventBus.new()

func init_game(world: World, ruleset: RulesetObject) -> void:
	_cleanup_nodes()
	
	current_game = Game.new(world, ruleset)
	event_bus = UiEventBus.new()
	
	_setup_nodes()


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
	_cleanup_nodes()
	event_bus = UiEventBus.new()
	current_game = Game.load_game(save_location, ruleset)
	if current_game == null:
		return ERR_BUG

	_setup_nodes()
	return OK
	
func _cleanup_nodes() -> void:
	if current_game != null and current_game.get_parent() != null:
		self.remove_child(current_game)
		current_game.queue_free()
		
	if event_bus != null and event_bus.get_parent() != null:
		self.remove_child(event_bus)
		event_bus.queue_free()
		
func _setup_nodes() -> void:
	assert (current_game != null)
	assert (event_bus != null)
	add_child(current_game)
	add_child(event_bus)
