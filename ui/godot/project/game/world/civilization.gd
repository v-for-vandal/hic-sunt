extends RefCounted

class_name Civilisation

## Result of the attempted city creation
## Only one variables will be filled.
class CreateCityResult:
	extends RefCounted
	
	var city: City
	var errors: PackedStringArray
	
	func is_ok() -> bool:
		return city != null

var _id: StringName
var _cities_by_id: Dictionary[StringName, City]
var _cities_by_region_id: Dictionary[StringName, City]

var _serializable_properties: Array[StringName] = ["_id",]

func id() -> StringName:
	return _id


func create_city(cell: CellObject) -> CreateCityResult:
	# Preconditions: CurrentGame and its components must be initialized
	assert(CurrentGame.game != null)
	assert(CurrentGame.game.world != null)
	assert(CurrentGame.current_game != null)
	assert(CurrentGame.current_game.session != null)
		
	var result := CreateCityResult.new()
		
	assert(cell != null)
	assert(cell.is_valid())
	if (cell == null) or (not cell.is_valid()):
		result.errors.append(tr(EK.ERR_INTERNAL_SELECTED_CELL_IS_INVALID))
		return result
		
	var region_id := cell.get_region_id()
	var region := CurrentGame.game.world.world_object.get_region_by_id(region_id)
	assert(region != null)
	if region == null:
		result.errors.append(tr(EK.ERR_INTERNAL_SELECTED_CELL_IS_INVALID))
		return result
	
	var city_scope : ScopeObject = CurrentGame.current_game.session.add_city(_id)
	if not city_scope:
		result.errors.append(tr(EK.ERR_INTERNAL_FAILED_TO_CREATE_SCOPE))
		return result
			
	var city_id := city_scope.get_id()
			
	var city := City.create_new_city(city_scope, cell)
		
	if not city:
		return result
		
	result.city = city
	_cities_by_id[city_id] = city
	_cities_by_region_id[region_id] = city
	# Register in World indices
	CurrentGame.game.world.register_city(city_id, region_id, city)
		
	print("Created city ", city_id, " in region ", region_id)
		
	assert(find_city_by_id(city_id) != null)
		
	CurrentGame.civ_propogate_ui_city_created(result.city)
		
	return result
	
func find_city_by_id(city_id: String) -> City:
	return _cities_by_id.get(city_id, null)
	
func can_create_city(cell: CellObject) -> bool:
	# check that this region is not under another civ control
	# TODO
	# check that we don't have a city here
	if cell.get_region_id() in _cities_by_region_id:
		return false
	
	return true
	
func next_turn() -> void:
	for city_id : String in _cities_by_id:
		_cities_by_id[city_id].next_turn()
	
static func create_civilisation(civ_id: String) -> Civilisation:
	# Preconditions: CurrentGame and its components must be initialized
	assert(CurrentGame.game != null)
	assert(CurrentGame.game.session != null)
	assert(CurrentGame.game.world != null)
		
	var civ_scope := CurrentGame.game.session.create_civilization_scope(civ_id)
	if not civ_scope:
		printerr("Failed to create civ with id: ", civ_id)
		return null
			
	var result := Civilisation.new()
	result._load_from_scope(civ_scope)
	# Register in World indices
	CurrentGame.game.world.register_civilisation(civ_id, result)
		
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

func _load_from_scope(civ_scope: ScopeObject):
	_cities_by_id.clear()
	self._id = civ_scope.get_id()
	
	
