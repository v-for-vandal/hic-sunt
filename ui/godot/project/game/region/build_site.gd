extends RefCounted

# Deprecated in favour of ConstructionProject

class_name BuildSite

var _id: int
var _improvement_id: String
var _cost : Dictionary
var _accumulated_resources: Dictionary
var _made_progress_this_turn := false
var _is_ready:= false
var _region: RegionObject
var _region_coords: Vector2i

static func CreateBuildSite(improvement_id: String, region: RegionObject, region_coords: Vector2i):
	var result := BuildSite.new()
	result._improvement_id = improvement_id
	result._cost = CurrentGame.get_ruleset().get_improvement_info(improvement_id).cost
	result._id = CurrentGame.get_new_id()
	result._region = region
	result._region_coords = region_coords
	
	return result
	
func id() -> int:
	return _id
	
func get_improvement_id() -> String:
	return _improvement_id
	
func _add_one_resource(resource_id: String, available_resources: Dictionary,
	available_resource_flow: Dictionary):
	if available_resources.get(resource_id, 0) <= 0:
		# Either we don't have this resoruce, or we are in debt (negative
		# value)
		return
	# how must do we need?
	var need : int = _cost.get(resource_id, 0) - _accumulated_resources.get(resource_id, 0)
	if need > 0:
		# try to take it from available_resources
		# TODO: For now we ignore resource flow
		var can_have : int = available_resources.get(resource_id, 0)
		var will_take : int = min(need, can_have)
		
		if will_take > 0:
			_accumulated_resources[resource_id] += will_take
			available_resources[resource_id] -= will_take
			_made_progress_this_turn = true
	
func add_resources(available_resources: Dictionary,
	available_resource_flow: Dictionary):
	# iterate over _cost

	for resource_id in _cost:
		_add_one_resource(resource_id, available_resources, available_resource_flow)
		
	var ready := true
	for resource_id: String in _cost:
		if _accumulated_resources.get(resource_id, 0) < _cost[resource_id]:
			ready = false
			break
	_is_ready = ready
	
		
func is_ready() -> bool:
	return _is_ready
	
func next_turn() -> void:
	# for now, we ignore build site degradation
	_made_progress_this_turn = false
	
