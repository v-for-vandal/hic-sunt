extends Node

signal progress_changed(progress, message)
signal load_done()

var _load_screen_path : String = "res://central/load_screen.tscn"
var _load_screen = load(_load_screen_path)

var _world_scene_resource : PackedScene = ResourceLoader.load("res://root_map.tscn")
var _loaded_resource: PackedScene
var _progress: Array = []
var use_subthreads = true
var _world_object : WorldObject
var _loading_screen_instance
var _new_scene

var _debug_timer: float = 0

func _ready() -> void:
	set_process(false)
	
func _prepare_loading()-> void:
	# switch to loading screen
	_loading_screen_instance = _load_screen.instantiate()
	get_tree().root.add_child(_loading_screen_instance)
	get_tree().current_scene = _loading_screen_instance
	
	# hide main screen
	get_tree().root.get_node('/root/MainScreen').visible = false
	_debug_timer = 0.0
	
	self.progress_changed.connect(_loading_screen_instance._update_progress_bar)
	self.load_done.connect(_loading_screen_instance._start_outro_animation)

# TODO: Move this function to CentralSystem
func _load_ruleset() -> RulesetObject:
	var core_ruleset_path = ProjectSettings.globalize_path('res://gamedata/v1.0')
	var _ruleset_dict : Dictionary = CentralSystem.load_ruleset(core_ruleset_path)
	# TODO: Process loading errors properly
	var _ruleset_object : RulesetObject
	if _ruleset_dict.success:
		print("Successfully loaded core ruleset: ", _ruleset_dict.success)
		_ruleset_object = _ruleset_dict.ruleset
	else:
		print("While loading core ruleset, there were errors: ", _ruleset_dict.errors)
	assert(_ruleset_object != null, "Failed to load ruleset")
	
	return _ruleset_object
	
func _init_world_scene() -> void:
	_new_scene = _world_scene_resource.instantiate()
	_new_scene.root_load()
	_new_scene.load_world(CurrentGame.current_world)
	
	set_process(true)
	
func _return_to_main_screen() -> void:
	if _new_scene != null:
		_new_scene.queue_free()
	var main_screen : Node = get_tree().root.get_node('/root/MainScreen')
	main_screen.visible = true
	get_tree().current_scene = main_screen
	
	if _loading_screen_instance != null:
		_loading_screen_instance.queue_free()
	
	
func new_game() -> void:
	
	_prepare_loading()
	
	var ruleset_object := _load_ruleset()
	
	var world_object : WorldObject = CentralSystem.create_world(Vector2i(10, 5), 7, ruleset_object)
	
	assert(world_object != null, "Failed to create world")

	CurrentGame.init_game(world_object, ruleset_object)
	
	_init_world_scene()

func load_game(savegame: String) -> void:

	_prepare_loading()
	
	var ruleset_object := _load_ruleset()
	
	# load save
	var status : Error = CentralSystem.load_game(savegame, ruleset_object)
	if status != OK:
		_return_to_main_screen()
		CentralSystem.error_report("Can't load savegame with code %s" % [error_string(status)])
		return
	# TODO: Check for errors from invocation above
	
	# Not exactly correct organization of invocations.
	# TODO: See where we shoud load things in async manner
	_init_world_scene()
	
#func start_load()-> void:
#
#
	#
#
		#
	#_world_object = CentralSystem.create_world(Vector2i(10, 5), 7, _ruleset_object)
	#
	#assert(_world_object != null, "Failed to create world")
#
	#CurrentGame.init_game(_world_object, _ruleset_object)

	
	
func _process(_delta) -> void:
	if _loading_screen_instance == null:
		set_process(false)
		return
	
	_debug_timer += _delta;
		
	if _debug_timer > 1.0:
	#if _world_object != null:
		set_process(false)
		emit_signal("progress_changed", 1.0)
		emit_signal("load_done") 
		# delete current world scene
		call_deferred("_defered_goto_scene")

		
func _defered_goto_scene() -> void:
		print("going to world scene")
		if has_node("/root/World"):
			var current_world_scene = get_node("/root/World")
			current_world_scene.queue_free()

		assert(_new_scene != null)
		get_tree().root.add_child(_new_scene)
		get_tree().current_scene = _new_scene

		if _loading_screen_instance != null:
			_loading_screen_instance.queue_free()
	
	
#func start_load() -> void:
	#var state = ResourceLoader.load_threaded_request(_scene_path, "",use_subthreads )
	#if state == OK:
		#set_process(true)
		#
#func _process(_delta):
	#var load_status = ResourceLoader.load_threaded_get_status(_scene_path, _progress)
	#match load_status:
		#0, 2: #? THREAD_LOAD_INVALID_RESOURCE, THREAD_LOAD_FAILED
			#set_process(false)
			#return
		#1: # THREAD_LOAD_IN_PROGRESS
			#emit_signal("progress_changed", _progress[0])
		#3: # THREAD_LOAD_LOADED
			#_loaded_resource = ResourceLoader.load_threaded_get(_scene_path)
			#emit_signal("progress_changed", 1.0)
			#emit_signal("load_done")
			#get_tree().change_scene_to_packed(_loaded_resource)
	#

