extends RefCounted

class_name Civilisation

var _id: int
var _cities_by_id: Dictionary
var _cities_by_region_id: Dictionary

func id() -> int:
	return _id
	
func create_city(region_id: String) -> City:
	assert(!region_id.is_empty())
	assert(can_create_city(region_id))
	if not can_create_city(region_id):
		return null
	var city_id : String = "city_%s" % CurrentGame.get_new_id()
	var result : City = City.create_new_city(city_id, region_id)
	assert(result != null)
	assert(not (city_id in _cities_by_id))
	if result == null:
		return result

	_cities_by_id[city_id] = result
	_cities_by_region_id[region_id] = result
	
	assert(CurrentGame.current_world.get_region_by_id(region_id) != null)
	CurrentGame.current_world.get_region_by_id(region_id).set_city_id(city_id)
	
	print("Created city ", city_id, " in region ", region_id)
	
	assert(find_city_by_id(city_id) != null)
	
	return result
	
func find_city_by_id(city_id: String):
	return _cities_by_id.get(city_id, null)
	
func can_create_city(region_id: String) -> bool:
	# check that this region is not under another civ control
	# TODO
	# check that we don't have a city here
	if region_id in _cities_by_region_id:
		return false
	
	return true
	
func next_turn():
	for city_id in _cities_by_id:
		_cities_by_id[city_id].next_turn()
	
static func create_civilisation() -> Civilisation:
	var civ_id = CurrentGame.get_new_id()
	var result = Civilisation.new()
	result._id = civ_id
	
	return result
	
	
