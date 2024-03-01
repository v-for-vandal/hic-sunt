extends RefCounted

class_name SerializeLibrary

static func serialize_to_variant(object: Object)->Dictionary:
	
	var serializable_properties : Array[StringName] = object.get_serializable_properties()
	var result := {}
	
	var object_properties := object.get_property_list()
	var object_properties_map = {}
	# build map
	for object_property_info in object_properties:
		object_properties_map[object_property_info.name] = object_property_info
		
	for property in serializable_properties:
		var property_info = object_properties_map[property]
		var property_value = object.get(property)
		
		if property_info.type == TYPE_OBJECT:
			result[property] = null if property_value == null else property_value.serialize_to_variant()
		else:
			result[property] = property_value
		
	return result
	
static func parse_from_variant(object: Object, data : Dictionary) -> void:
	assert(object != null)
	
	var serializable_properties : Array[StringName] = object.get_serializable_properties()
	
	var object_properties := object.get_property_list()
	var object_properties_map = {}
	# build map
	for object_property_info in object_properties:
		object_properties_map[object_property_info.name] = object_property_info
		
	for property in serializable_properties:
		var property_info = object_properties_map[property]
		var property_value = data[property]
		
		if property_info.type == TYPE_OBJECT:
			if property_value == null:
				object.set(property, null)
			else:
				object.get(property).parse_from_variant(property_value)
		else:
			object.set(property, property_value)

