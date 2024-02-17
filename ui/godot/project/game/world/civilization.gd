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
	var city_id = CurrentGame.get_new_id()
	var result = City.create_new_city(city_id, region_id)
	assert(not (city_id in _cities_by_id))
	_cities_by_id[city_id] = result
	_cities_by_region_id[region_id] = result
	
	return result
	
func can_create_city(region_id: String) -> bool:
	# check that this region is not under another civ control
	pass
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
	
	
