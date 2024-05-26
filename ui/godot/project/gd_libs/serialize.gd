extends RefCounted

class_name SerializeLibrary

static func _serialize_region_object(region: RegionObject) -> Dictionary:
	assert(region != null)
	return {
		&"$type" : &"core.type.region",
		&"region_id" : region.get_region_id()
	}
	
static func _parse_region_object(data: Dictionary) -> RegionObject:
	var region_id = data.get("region_id", "") 
	if region_id == null or region_id.is_empty():
		push_error("Region id is missing in serialized data")
		return null
	
	# Get this region from current world
	var region = CurrentGame.get_current_world().get_region_by_id(region_id)
	if region == null:
		push_error("Region with id ", region_id, " is not present in current world")
		
	# may be null
	return region

static func serialize_to_variant(object: Object)->Dictionary:
	
	var serializable_properties : Array[StringName] = object.get_serializable_properties()
	var result := {}
	
	var object_properties := object.get_property_list()
	var object_properties_map = {}
	# build helper map
	for object_property_info in object_properties:
		object_properties_map[object_property_info.name] = object_property_info
		
	for property in serializable_properties:
		var property_info = object_properties_map[property]
		var property_value = object.get(property)
		
		if property_value == null:
			result[property] = null
			continue
		match property_info.type:
			TYPE_OBJECT:
				# Handle special types first
				if property_value is RegionObject:
					result[property] = _serialize_region_object(property_value)
				else:
					# ordinary case
					result[property] = property_value.serialize_to_variant()
			TYPE_ARRAY, TYPE_DICTIONARY:
				result[property] = property_value
			TYPE_STRING, TYPE_STRING_NAME:
				result[property] = property_value
			TYPE_INT, TYPE_FLOAT, TYPE_BOOL:
				result[property] = property_value
			_:
				result[property] = var_to_str(property_value)

		
	return result
	
static func parse_from_variant(object: Object, data : Dictionary) -> void:
	assert(object != null)
	
	var serializable_properties : Array[StringName] = object.get_serializable_properties()
	
	var object_properties := object.get_property_list()
	var object_properties_map = {}
	# build map property name -> property info
	for object_property_info in object_properties:
		object_properties_map[object_property_info.name] = object_property_info
		
	for property in serializable_properties:
		if property not in data:
			# for compatibility, skip things that are not present in
			# serialization
			continue
		var property_info = object_properties_map[property]
		var property_value = data[property]
		
		match property_info.type:
			TYPE_OBJECT:
				if property_value == null:
					object.set(property, null)
					continue
				
				# Check for special types first
				if property_value is Dictionary:
					if property_value.has("$type"):
						var type = property_value.get("$type", "")
						if type == &"core.type.region":
							# this is region object
							object.set(property, _parse_region_object(property_value))
							continue
				# Ordinary object, straightforward handling
				assert(object.get(property) != null,
					'for serialization to work property must contain an object')
				# TODO: we can actually work around, at least for typed properties by creating object
				# with Type.new()
				object.get(property).parse_from_variant(property_value)
				
			TYPE_STRING, TYPE_STRING_NAME:
				object.set(property, property_value)
			TYPE_DICTIONARY, TYPE_ARRAY:
				# TODO: Can we work with typed arrays ?
				object.set(property, property_value)
			TYPE_INT, TYPE_FLOAT, TYPE_BOOL:
				object.set(property, property_value)
			_:
				assert(property_value is String, "Unsupported deserialization from non-string type")
				var converted = str_to_var(property_value)
				object.set(property, converted)
							
				
			

# method returns class name that can be later used to make '.new()'
static func _get_class_name(object: Object) -> String:
	return ""
	pass
	
# This helper function is required with you have an array of pointers to base
# classes. E.g. 
# class_name Base
# class_name Derived1; extends Base
# class_name Derived2; extends Base
# your data is:
# data : Array[Base], but inside there is a mix of Derived1, Derived2, ...
# and you need to serialize and deserialize it as is
static func serialize_array_with_types(data: Array[Object])->Array[Dictionary]:
	for elem in data:
		pass
	return []
		

