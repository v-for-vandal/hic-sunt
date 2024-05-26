extends RefCounted

## This is queue of all city projects currently active. Player can rearrange
## projects, pause them and more.

class_name CityProjectsQueue

# public signals

signal removed(idx: int)
signal swapped(idx1 : int, idx2: int)
signal inserted(idx: int)

# private members
var _queue : Array[CityProject]

var _serializable_properties := [
	# currently no property is auto-serializable
]

# == Organizing queue ==
func size() -> int:
	return _queue.size()
	
func get_project_at(idx: int) -> CityProject:
	return _queue[idx]
	
func add_project(project: CityProject, index: int) -> void:
	_queue.insert(index, project)
	inserted.emit(index)
	
func clear() -> void:
	var size := _queue.size()
	_queue = []
	var idx = size - 1
	while idx >= 0:
		removed.emit(idx)
		idx -= 1
	
#func rearrange(source_idx: int, target_idx: int) -> void:
#	var prj = _queue[source_idx]
	
func swap(source_idx: int, target_idx: int) -> void:
	if target_idx > _queue.size() or target_idx < 0:
		push_error("target_idx is out of bounds: ", target_idx)
		return
	if source_idx > _queue.size() or source_idx < 0:
		push_error("source_idx is out of bounds: ", source_idx)
		return
		
	var tmp : CityProject = _queue[source_idx]
	_queue[source_idx] = _queue[target_idx]
	_queue[target_idx] = tmp
	swapped.emit(source_idx, target_idx)
	

# == processing ==
func process(available_resources: Dictionary) -> void:
	for project in _queue:
		if project.is_finished():
			## It should have been cleared on previous iteration,
			## but things happen. May be some magical spell 'finish current
			## building' was called etc etc.
			project.execute_finisher()
			continue
		if not project.is_possible():
			project.execute_skipped()
			
		var was_changed := project.take_resources(available_resources)
		if was_changed:
			project.changed.emit()
		
		if project.is_finished():
			project.execute_finisher()
			
	# now, clean up finished projects
	var size := _queue.size()
	var idx = size - 1
	while idx >= 0:
		if _queue[idx].is_finished():
			removed.emit(idx)
			_queue.remove_at(idx)
		idx -= 1

func get_serializable_properties() -> Array[StringName]:
	return _serializable_properties

func serialize_to_variant() -> Dictionary:
	# because _queue is duck-typed, we have to handle it manually
	var result : Array[Variant] = []
	for project : CityProject in _queue:
		var project_type := project.get_project_type()
		var data := {}
		data["$type"] = project_type
		data["data"] = project.serialize_to_variant()
		result.append(data)
	return {
		# array of dicts, each dict is a project
		"queue" : result
	}
	
func parse_from_variant(data : Dictionary) -> void:
	clear()
	var queue : Array = data.get("queue", []) 
	for project in queue:
		# deserialize project
		var project_type : String = project.get("$type", "")
		if project_type.is_empty():
			push_error("Missing project type when deserializing. Skipping")
			continue
		
		if "data" not in project:
			push_error("Invalid serialized object, missing field 'data'")
			continue

		var project_data = project["data"]

		# find project with this type and its gdscript
		var project_type_info : Dictionary = CurrentGame.get_current_player_ruleset().get_project_info(project_type)
		if project_type_info.is_empty():
			push_error("Failed to find project type: ", project_type_info)
			continue
		
		# Now, get its script
		var script_path: String = project_type_info.get("script", "")
		if script_path.is_empty():
			push_error("Project must specify script")
			continue
		
		# Try to get this script
		var script : Script = load(script_path)
		if script == null:
			push_error("Failed to load script ", script_path, " for project type: ", project_type)
			continue
			
		if not script.can_instantiate():
			push_error("Script: ", script_path, " for project: ", project_type, " can not be instantiated" )
			continue
			
		# instantiate script
		var project_object = script.new()
		if project_object == null:
			push_error("Failed to instantiate script: ", script_path, " for project: ", project_type)
			continue
			
		# deserialize
		project_object.parse_from_variant(project_data)
		
		# add to queue
		_queue.append(project_object)
		

