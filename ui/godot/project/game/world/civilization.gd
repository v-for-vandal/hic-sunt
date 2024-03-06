extends RefCounted

class_name Civilisation

var _id: int
var _cities_by_id: Dictionary
var _cities_by_region_id: Dictionary

var _serializable_properties: Array[StringName] = ["_id",]

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
	
func find_city_by_id(city_id: String) -> City:
	return _cities_by_id.get(city_id, null)
	
func can_create_city(region_id: String) -> bool:
	# check that this region is not under another civ control
	# TODO
	# check that we don't have a city here
	if region_id in _cities_by_region_id:
		return false
	
	return true
	
func next_turn() -> void:
	for city_id : String in _cities_by_id:
		_cities_by_id[city_id].next_turn()
	
static func create_civilisation() -> Civilisation:
	var civ_id : int = CurrentGame.get_new_id()
	var result := Civilisation.new()
	result._id = civ_id
	
	return result
	
func get_serializable_properties() -> Array[StringName]:
	return _serializable_properties
	
func serialize_to_variant() -> Dictionary:
	# serialize plain properties with helper
	var result := SerializeLibrary.serialize_to_variant(self)
	
	# serialize cities
	var cities := {}
	for city_id : String in _cities_by_id:
		cities[city_id] = _cities_by_id[city_id].serialize_to_variant()
		
	result["cities"] = cities
	
	return result

	
func parse_from_variant(data : Dictionary) -> void:
	#_id = data["id"]
	_cities_by_id.clear()
	_cities_by_region_id.clear()
	
	# load base properties
	SerializeLibrary.parse_from_variant(self, data)
	
	var cities_data: Dictionary = data["cities"]
	# restor cities map map
	print("restoring cities ", cities_data)
	for city_id: String in cities_data:
		var city :=  City.new()
		city.parse_from_variant(cities_data[city_id])
		_cities_by_id[city_id] = city;
		_cities_by_region_id[city.get_region_id()] = city

	
	
	
