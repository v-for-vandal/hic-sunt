extends Node

func _ready() -> void:
	_register_dev_console_commands()

func _register_dev_console_commands()-> void:
	Console.create_command("build", self._dev_build , "Immediatelly build specified improvement")
	pass
	
func _dev_build(improvement_id = null, a = 1, b = "s", f = {}):
	if improvement_id == null:
		# create a popup
		var popup_menu := PopupMenu.new()
		var ruleset : RulesetObject = CurrentGame.get_current_player_ruleset()
		var available_buildings : Array = ruleset.get_all_region_improvements()
	
		for bld : Variant in available_buildings:
			popup_menu.add_item(bld.id)
			
		$CanvasLayer.add_child(popup_menu)
		popup_menu.popup_centered()
		
		var idx : int = await popup_menu.index_pressed
		
		$CanvasLayer.remove_child(popup_menu)
		popup_menu.queue_free()
		
		improvement_id = available_buildings[idx]
	print("Building an improvement: ", improvement_id)
	
	
func _input(event: InputEvent) -> void:
	# We have to catch this key before _gui_input, otherwise
	# console itself will catch it and simply print as a character
	if event.is_action_pressed("ui_dev_console"):
		$%DevConsole.visible = !$%DevConsole.visible
		get_viewport().set_input_as_handled()
