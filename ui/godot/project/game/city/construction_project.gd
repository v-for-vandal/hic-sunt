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
var _progress := 0
var _turns_left := -1

var _serializable_properties: Array[StringName] = [
	"_id",
	"_improvement_id",
	"_turns_without_progress",
	"_is_ready",
	"_region",
	"_region_coords",
	"_progress",
	"_turns_left"
	]

# TODO: Rename get_type_id
static func get_project_type() -> StringName:
	return &"core.prj.construction"

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
var _total_accumulated_resources := 0 # sum of all elements in _accumulated_resources
var _total_required_resources := 0 # sum of all resources required for completition

static func create_construction_project(improvement_id: StringName, region: RegionObject,
	region_coords: Vector2i) ->ConstructionProject:
	assert(region != null, "Null region object")
	
	if not region.contains(region_coords):
		push_error("Region with id ", region.get_region_id() , " doesn't contains coords ", region_coords)
		return null
	
	var result := ConstructionProject.new()
	result._improvement_id = improvement_id
	var improvement_info : Dictionary = CurrentGame.get_ruleset().get_improvement_info(improvement_id)
	if improvement_info.is_empty():
		push_error("No information for improvement: ", improvement_id)
		return null

	result._resources_cost = improvement_info.cost
	result._id = CurrentGame.get_new_id()
	result._region = region
	result._region_coords = region_coords
	result._total_required_resources = _calc_total_required(result._resources_cost)
	
	# setup accumulated resources with zeroes
	for resource_id in result._resources_cost:
		result._accumulated_resources[resource_id] = 0
	result._accumulated_resources[CityConstants.WORKFORCE_RESOURCE] = 0
	
	# TODO: We should be able to estimeate 'turns_left' without waiting
	# for next turn
	
	return result
	
	
func id() -> int:
	return _id

func get_improvement_id() -> String:
	return _improvement_id
	
static func _calc_total_required(resources_cost : Dictionary) -> int:
	var result := 0
	for resource_id in resources_cost:
		if resource_id == CityConstants.WORKFORCE_RESOURCE:
			result += resources_cost[resource_id]
		else:
			# for any resource other then workforce, we require
			# both the resource (stone, wood, etc) and workforce,
			# in equal measure
			result += 2 * resources_cost[resource_id]
	return result
		
	
## Returns true if requirement for this resource was fully satistfied - that is
## we accumulated as much as we needed.
func _take_one_resource(resource_id: String, available_resources: Dictionary) -> bool:
	print("Taking ", resource_id) # TODO: RM
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
			# *2 because we took both resource and workforce, equal amount
			_total_accumulated_resources += 2 * will_take
			
			print("Took ", will_take) # TODO: RM
			
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
	
	
# return true if the was any change in status
func _take_resources(available_resources: Dictionary) -> bool:
	# maximum number of resources taken is 'workforce'
	var max_workforce : int = available_resources.get(CityConstants.WORKFORCE_RESOURCE, 0)
	
	if max_workforce <= 0:
		# There is no workforce left
		return false
	# iterate over _cost

	var prev_total_accumulated_resources := _total_accumulated_resources
	
	var all_resources_accumulated := true
	for resource_id : String in _resources_cost:
		# skip assembly cost
		if resource_id == CityConstants.WORKFORCE_RESOURCE:
			continue
		
		var finished_with_this_resource := _take_one_resource(resource_id, available_resources)
		if not finished_with_this_resource:
			all_resources_accumulated = false

	var ready := all_resources_accumulated

	if all_resources_accumulated:
		# if all resources are collected, we can charge 'assembly cost', if any
		if CityConstants.WORKFORCE_RESOURCE in _resources_cost:
			var finished_with_assembly := _take_assembly_cost(available_resources)
			if not finished_with_assembly:
				ready = false
	
	_is_ready = ready
	
	if _total_required_resources != 0:
		_progress = int(float(_total_accumulated_resources) / _total_required_resources * 100)
		
	if _is_ready:
		_progress = 100
		
	# return whether the was some change
	return (prev_total_accumulated_resources != _total_accumulated_resources)
	

func is_finished() -> bool:
	return _is_ready
	
func take_resources(resources: Dictionary) -> bool:
	var status := _take_resources(resources)
	if not status:
		_turns_without_progress += 1
		
	return status
	
func progress_estimate() -> Dictionary:
	return {
		"progress" : _progress,
		"turns_left" : _turns_left,
		"turns_without_progress" : _turns_without_progress
	}
	
func is_possible() -> bool:
	# At the moment we make no checks
	return true
	
func execute_finisher() -> bool:
	if not _region.set_improvement(_region_coords, _improvement_id):
		push_error("Failed to set improvement at region ", _region.get_region_id())
		return false
	return true
	
func execute_skipped() -> bool:
	_turns_without_progress += 1
	return true
	
func _clear():
	_id = 0
	_improvement_id = ""

	_turns_without_progress = 0
	_is_ready = false
	_region = null
	_region_coords = Vector2i(0,0)
	_progress = 0
	_turns_left = 0
	
func get_serializable_properties() -> Array[StringName]:
	return _serializable_properties

func serialize_to_variant() -> Dictionary:
	return SerializeLibrary.serialize_to_variant(self)
	
func parse_from_variant(data : Dictionary) -> void:
	_clear()
	SerializeLibrary.parse_from_variant(self, data)
	
