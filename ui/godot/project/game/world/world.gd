extends RefCounted

class_name World

var _world_object: WorldObject
var _planes : Dictionary[StringName, WorldPlane]

var world_object: WorldObject:
	get: return _world_object
	
func _init():
	_world_object = WorldObject.new()

func create_plane(name: StringName, world_size : Rect2i, region_radius : int, region_external_radius: int = -1) -> WorldPlane:
	var plane_object :=  _world_object.create_plane(name, world_size, region_radius, region_external_radius)
	if plane_object != null:
		var plane : WorldPlane = WorldPlane.new(plane_object)
		_planes[name] = plane
		return plane
	else:
		push_error("failed to create Plane")
		return null
		
func get_plane(plane_id: StringName) -> WorldPlane:
	if plane_id in _planes:
		return _planes[plane_id]
		
	# We can not load data this way because there is nothing to load. The code
	# below is ONLY to put proper warning
	var plane_object := _world_object.get_plane(plane_id)
	if plane_object != null:
		push_error("You have requested plane {} that exists in underlying WorldObject, but not in our class. This is a logic error and it shoud not have happend" % plane_id)
		
	return null

## Saves world to given location
func save(path: String) -> Error:
	return _world_object.save(path)

## Loads world from given location
func load(path: String) -> Error:
	return _world_object.load(path)
	
# TODO:
#func get_plane(...)
