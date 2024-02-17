extends RefCounted

class_name Civilisation

var _id: int
var _cities: Dictionary

func id() -> int:
	return _id
	
func create_city(qr_coords: Vector2i):
	var city_id = CurrentGame.get_new_id()
	var result = City.create_new_city(city_id, qr_coords)
	assert(not (city_id in _cities))
	_cities[city_id] = result
	
	return result
	
func next_turn():
	for city_id in _cities:
		_cities[city_id].next_turn()
	
static func create_civilisation() -> Civilisation:
	var civ_id = CurrentGame.get_new_id()
	var result = Civilisation.new()
	result._id = civ_id
	
	return result
	
	
