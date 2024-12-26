# we must extend Node, otherwise Autoload wont work
extends Node

var _world_generators : Dictionary = {}

## Register a module for use in world builder subsystem
func register_module(mod_info: ModInfo, module: Script) -> bool:
	# modconfig is config for whole mod. One mod may have a lot of 
	# create new instance of the script
	var instance : Object = module.new()
	
	if instance is not RefCounted:
		push_error("Module must inherit RefCounted")
		instance.free()
		return false
		
	if not instance.has_method('get_module_info'):
		push_error("Module lacks required method get_module_info")
		return false
		
	var module_info : Dictionary = (instance.get_module_info() as Dictionary)
	if module_info == null:
		push_error("Failed to get module information")
		return false
		
	if instance.has_method('create_world'):
		# Ok, this is world generator module
		_world_generators[mod_info.name] = module

	
	return true
	
