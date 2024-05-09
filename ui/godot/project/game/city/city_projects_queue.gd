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

# == Organizing queue ==
func size() -> int:
	return _queue.size()
	
func get_project_at(idx: int) -> CityProject:
	return _queue[idx]
	
func add_project(project: CityProject, index: int) -> void:
	_queue.insert(index, project)
	inserted.emit(index)
	
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

