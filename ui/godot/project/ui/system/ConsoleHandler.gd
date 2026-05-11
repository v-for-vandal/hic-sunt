extends Node

var DevSelectAndBuildInteraction := preload("res://game/interactions/dev/dev_select_and_build.gd")

var _stats_dumpers := {
	'effects' : self._dump_statistics_effects
}

func _ready() -> void:
	_register_dev_console_commands()

func _register_dev_console_commands()-> void:
	Console.create_command("build", self._dev_build , "Immediatelly build specified improvement")
	Console.create_command("next_turn", CurrentGame.next_turn, "Start next turn")
	Console.create_command("dump_statistics", self._dump_statistics, "Dump specified statistics to log. Statistics type: %s" % [_stats_dumpers.keys()])
	
func _dev_build(improvement_id : Variant = null) -> void:
	var resolved_improvment_id : String
	if improvement_id == null:
		# create a popup
		var popup_menu := PopupMenu.new()
		var ruleset : RulesetObject = CurrentGame.current_game.ruleset
		var available_buildings : Array = ruleset.get_all_region_improvements()
	
		for bld : Variant in available_buildings:
			popup_menu.add_item(bld.id)
			
		$CanvasLayer.add_child(popup_menu)
		popup_menu.popup_centered()
		
		var idx : int = await popup_menu.index_pressed
		
		$CanvasLayer.remove_child(popup_menu)
		popup_menu.queue_free()
		
		resolved_improvment_id = available_buildings[idx].id as String
	else:
		resolved_improvment_id = improvement_id as String
		
	var new_build_interaction := DevSelectAndBuildInteraction.new(resolved_improvment_id)
	CurrentGame.event_bus.set_main_interaction(new_build_interaction)
	
func _dump_statistics(statistics_type: String) -> void:
	if statistics_type not in _stats_dumpers:
		Console.print_line("Unknown statistics %s. Valid statistics are: %s" % [statistics_type, _stats_dumpers.keys()])
		return
		
	var stat_group := CurrentGame.current_game.debug_control.add_group("statistics")
	var effect_node : DebugTree.TreeControl = stat_group.find_node(statistics_type)
	if effect_node == null:
		effect_node = stat_group.add_text_node(statistics_type, "")
	
	_stats_dumpers[statistics_type].call(effect_node)
	
func _dump_dict_to_text_node(target: Dictionary, debug_node: DebugTree.TreeControl) -> void:
	for key: Variant in target:
		debug_node.add_text("%-32s: %10s" % [key, target[key]])
	
func _dump_statistics_effects(debug_node: DebugTree.TreeControl) -> void:
	debug_node.add_text("Total statistics\n")
	var total_stats := CurrentGame.current_game.session.get_total_effect_execution_statistics()
	_dump_dict_to_text_node(total_stats, debug_node)
		
	debug_node.add_text("Turn statistics\n")
	var turn_stats := CurrentGame.current_game.session.get_last_effect_execution_statistics()
	_dump_dict_to_text_node(turn_stats, debug_node)
	
func _input(event: InputEvent) -> void:
	# We have to catch this key before _gui_input, otherwise
	# console itself will catch it and simply print as a character
	if event.is_action_pressed("ui_dev_console"):
		$%DevConsole.visible = !$%DevConsole.visible
		get_viewport().set_input_as_handled()
