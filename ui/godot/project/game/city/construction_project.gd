extends CityProject

## Construction project is a city project that builds some improvement.
## They require both resources and workforce, and they must take resources
## and workforce in proportion 1-to-1.
## That is if improvement costs 5 wood, 10 stone, it will require 10+5 workforce
## as well. And if you have 100 wood, 100 stone, 5 workforce in pool, you
## can only take 5 workforce and 5 resources (combined). Essentially, workforce
## is required to 'move resources/use resources'.
## Similarily, if you have 1 wood, 1 stone, 100 workforce in pool, you can't
## just take 1 wood, 1 stone and 15 workforce - this is not a modern economy:)
## You can't spend workforce in advance, and then receive resources ))
## However, we support situation where some additional workforce is required,
## for example 'to assemble' the building. If workforce cost is greater then
## sum of all resources, we will charge it at the end as simple resource.

class_name ConstructionProject

# private variables
var _id: int
var _improvement_id: String

var _turns_without_progress := 0
var _is_ready:= false
var _region: RegionObject
var _region_coords: Vector2i

# We have two maps - one with resources cost, one with accumulated resources
# instead of one map like 'remaining_required_resources_to_finish' because
# we want so support build site degradation - where if left unfinished,
# resources slowly dwindle (wood rots, stone breaks under wind, I guess, and so
# on)
# _resources_cost may contain 'WORKFORCE_RESOURCE', in this case this is considered
# 'assembly cost' and will be treated somewhat like normal resource. See comment
# for the class.
var _resources_cost : Dictionary
var _accumulated_resources: Dictionary


static func CreateConstructionProject(improvement_id: String, region: RegionObject, region_coords: Vector2i):
	var result := ConstructionProject.new()
	result._improvement_id = improvement_id
	result._cost = CurrentGame.get_ruleset().get_improvement_info(improvement_id).cost
	result._calculate_workforce_cost()
	result._id = CurrentGame.get_new_id()
	result._region = region
	result._region_coords = region_coords
	
	return result
	
func id() -> int:
	return _id
	

func get_improvement_id() -> String:
	return _improvement_id
	
## Returns true if requirement for this resource was fully satistfied - that is
## we accumulated as much as we needed.
func _take_one_resource(resource_id: String, available_resources: Dictionary) -> bool:
	if available_resources.get(resource_id, 0) <= 0:
		# Either we don't have this resoruce, or we are in debt (negative
		# value)
		return _accumulated_resources.get(resource_id, 0) == _resources_cost.get(resource_id,0)
		
	assert(resource_id != CityConstants.WORKFORCE_RESOURCE,
		"you can't use this method to charge workforce")
	
		
	# how much do we need?
	var need : int = _resources_cost.get(resource_id, 0) - _accumulated_resources.get(resource_id, 0)
	if need > 0:
		# try to take it from available_resources
		# TODO: For now we ignore resource flow
		var workforce_remaining : int = available_resources.get(CityConstants.WORKFORCE_RESOURCE)
		var resource_remaining : int = available_resources.get(resource_id, 0)
		var can_have : int = min(resource_remaining,  workforce_remaining)
		var will_take : int = min(need, can_have)
		
		if will_take > 0:
			_accumulated_resources[resource_id] += will_take
			available_resources[resource_id] -= will_take
			_accumulated_resources[CityConstants.WORKFORCE_RESOURCE] += will_take
			available_resources[CityConstants.WORKFORCE_RESOURCE] -= will_take
			_turns_without_progress = 0
			
		return need == will_take
	
	# need is 0, so true
	return true
			
func _take_assembly_cost(available_resources: Dictionary) -> bool:
	var need : int = (_resources_cost.get(CityConstants.WORKFORCE_RESOURCE, 0) -
		_accumulated_resources.get(CityConstants.WORKFORCE_RESOURCE, 0))
		
	if need > 0:
		# try to take it from available_resources
		var workforce_remaining : int = available_resources.get(CityConstants.WORKFORCE_RESOURCE)
		var will_take : int = min(need, workforce_remaining)
		
		if will_take > 0:
			_accumulated_resources[CityConstants.WORKFORCE_RESOURCE] += will_take
			available_resources[CityConstants.WORKFORCE_RESOURCE] -= will_take
			_turns_without_progress = 0
			
		return will_take == need
		
	# need is 0, so true
	return true
	
	
func _take_resources(available_resources: Dictionary):
	# maximum number of resources taken is 'workforce'
	var max_workforce : int = available_resources.get(CityConstants.WORKFORCE_RESOURCE, 0)
	
	if max_workforce <= 0:
		# There is no workforce left
		return
	# iterate over _cost

	var all_resources_accumulated := true
	for resource_id : String in _resources_cost:
		# skip assembly cost
		if resource_id == CityConstants.WORKFORCE_RESOURCE:
			continue
		var finished_with_this_resource := _take_one_resource(resource_id, available_resources)
		if not finished_with_this_resource:
			all_resources_accumulated = false
		


	#for resource_id: String in _resources_cost:
		#if resource_id == CityConstants.WORKFORCE_RESOURCE:
			## assembly cost is charged at the end
			#continue;
		#if _accumulated_resources.get(resource_id, 0) < _resources_cost[resource_id]:
			#all_resources_accumulated = false
			#break
	

	var ready := all_resources_accumulated

	if all_resources_accumulated:
		# if all resources are collected, we can charge 'assembly cost', if any
		if CityConstants.WORKFORCE_RESOURCE in _resources_cost:
			var finished_with_assembly := _take_assembly_cost(available_resources)
			if not finished_with_assembly:
				ready = false
	
	_is_ready = ready
	

# City project api
func turns_without_progress() -> int:
	return _turns_without_progress
	
func is_finished() -> bool:
	return _is_ready
	
func take_resources(resources: Dictionary) -> void:
	_take_resources(resources)
	
