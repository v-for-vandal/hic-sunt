# Global state that manages current game and its logic
extends Node

## Signal for UI that some region has changed and ui must be refreshed. These signals are unsuitable
## for game logic. There is no guarantee that every change made will invoke this signal.
signal ui_region_changed(region: RegionObject)

## Signal for UI that some cell has changed and ui must be refreshed. These signals are unsuitable
## for game logic. There is no guarantee that every change made will invoke this signal.
signal ui_cell_changed(cell: CellObject)

## Signal for UI that city was created.
## These signals are unsuitable  for game logic. There is no guarantee that every change 
## made will invoke this signal.
signal ui_city_created(city: City)

var current_game: Game
var event_bus : UiEventBus = UiEventBus.new()

# just an alias
var game: Game:
	get:
		return current_game

func init_game(world: World, ruleset: RulesetObject) -> void:
	_cleanup_nodes()
	
	current_game = Game.new(world, ruleset)
	event_bus = UiEventBus.new()
	
	_setup_nodes()
	
	current_game.setup()

## Replaces ruleset in running game. Potentialy dangerous operation
func replace_ruleset(ruleset: RulesetObject) -> void:
	if current_game == null:
		push_error("Attempt to replace ruleset while no current game is present")
		return
		
	current_game.replace_ruleset(ruleset)


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
	current_game.session.region_changed.connect(_propogate_ui_region_changed)
	current_game.session.cell_changed.connect(_propogate_ui_cell_changed)
	
# Signals propogator
func _propogate_ui_region_changed(region: RegionObject) -> void:
	ui_region_changed.emit(region)
		
func _propogate_ui_cell_changed(cell: CellObject) -> void:
	ui_cell_changed.emit(cell)
	
# This method is called by Civilization class to notify that city was created
func civ_propogate_ui_city_created(city: City) -> void:
	ui_city_created.emit(city)
