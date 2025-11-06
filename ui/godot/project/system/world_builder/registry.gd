# we must extend Node, otherwise Autoload wont work
extends Node


## Base class for handles
class HandleBase:
	var name: String
	## loaded script. Store ref to it to ensure its lifetime
	var _module_script: Script
	## Instance of a module. Callables will point here
	var _instance: RefCounted
	## Target module function.
	## Signature:
	## (Dictionary[String, Variant]) -> WorldGeneratorInterface
	var _generator_function : Callable
	## This function returns UI control node. Do not use it directly,
	## use create_ui function
	var _generator_ui_function : Callable
	
	func has_ui() -> bool:
		return _generator_ui_function != null

## This class is a handle to world generator. It encapsulates whole process
## of creating world
class WorldGeneratorHandle:
	extends HandleBase
	
	## Create generator
	func create_generator( config: Variant, debug_control: DebugTree.ControlInterface) -> WorldGeneratorInterface:
		assert(_generator_function != null)
		return _generator_function.call(config, debug_control)
	
	## Create Control node for this generator. Control node must have additional
	## method get_config. The result is passed to create_generator method.
	func create_ui() -> Control:
		if _generator_ui_function != null:
			return self._generator_ui_function.call()
		# null is a valid option if module has no UI element
		return null


## This class is a handle to worldgen module. They usually desrcibe a generator
## for one stage in pipeline - like heighmap generator, or river generator
## and so on.
## However, whole world generators also use this interface.
class WorldGeneratorModuleHandle:
	extends HandleBase

	
	## Create generator
	func create_generator(plane: PlaneObject, config:Variant, global_context: WorldGeneratorGlobalContext) -> WorldGeneratorModuleInterface:
		assert(_generator_function != null)
		return _generator_function.call(plane, config, global_context)
	
	## Create Control node for this generator. Control node must have additional
	## method get_config. The result is passed to create_generator method.
	func create_ui() -> Control:
		if _generator_ui_function != null:
			return self._generator_ui_function.call()
		# null is a valid option if module has no UI element
		return null
		

		


var _world_generators_by_name : Dictionary[String, WorldGeneratorHandle] = {}
var _worldgen_modules_by_category: Dictionary[WorldBuilderRegistry.CATEGORY, Array] = {}

## mapping from category to its function name 
var _module_function_mapping : Dictionary[WorldBuilderRegistry.CATEGORY, StringName]= {
	WorldBuilderRegistry.CATEGORY.Heightmap : &'create_heightmap_generator',
	WorldBuilderRegistry.CATEGORY.Climate : &'create_climate_generator',
	WorldBuilderRegistry.CATEGORY.Biome : &'create_biome_generator',
}

enum CATEGORY {
	Heightmap,
	Climate,
	Biome,
}


		
## Return list for all world generators
func get_world_generators() -> Array[WorldGeneratorHandle]:
	return _world_generators_by_name.values()

## Return list of all enabled modules for selected category
func get_modules_for_category(category: CATEGORY) -> Array[WorldGeneratorModuleHandle]:
	var result : Array[WorldGeneratorModuleHandle]
	result.assign(_worldgen_modules_by_category.get(category, []))
	return result

## Register a module for use in world builder subsystem
func register_module(mod_info: ModInfo, module: Script) -> bool:
	assert(mod_info.is_valid())
	
	print('Worldgen: loading module: %s' %  module.resource_path)

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
		
	var module_name : String = module_info.get(&'name', mod_info.name)
	print('Module name: %s' % module_name)
		
		
	if instance.has_method('create_world_generator'):
		print('Found world generator')
		var handle := _create_generator_handle(module, module_name, 'create_world_generator')
		# Ok, this is world generator module
		_world_generators_by_name[module_name] = handle
		
	# Load modules for every category
	for category  in _module_function_mapping:
		var function_name := _module_function_mapping[category]
		if instance.has_method(function_name):
			print('Found %s generator' % WorldBuilderRegistry.CATEGORY.keys()[category])
			var handle := _create_module_handle(module, module_name, function_name)
			_worldgen_modules_by_category.get_or_add(category, []).append(handle)
		
	print('Success')
	return true
	
## Do checks in advance!
func _create_module_handle(module: Script, module_name: String, base_method_name: String) -> WorldGeneratorModuleHandle:
	# We create instance every time to prevent misuse - different generators
	# should not communicate via script local variables, because there is
	# no guarantee that, say, heightmap and climate generators will come
	# from same module. So climate generator may discover, to its surprise,
	# that some local data was never initialized because some previous
	# generator from same script was never called.
	var instance : Object = module.new()
	assert(instance.has_method(base_method_name))

	var handle := WorldGeneratorModuleHandle.new()
	handle.name = module_name
	handle._module_script = module
	handle._instance = instance
	handle._generator_function = Callable.create(instance, base_method_name)
	
	var ui_method_name := base_method_name + "_ui"
	if instance.has_method(ui_method_name):
		handle._generator_ui_function = Callable.create(instance, ui_method_name)
		
	return handle
	
func _create_generator_handle(module: Script, module_name: String, base_method_name: String) -> WorldGeneratorHandle:
	
	# We create instance every time to prevent misuse - different generators
	# should not communicate via script local variables, because there is
	# no guarantee that, say, heightmap and climate generators will come
	# from same module. So climate generator may discover, to its surprise,
	# that some local data was never initialized because some previous
	# generator from same script was never called.
	var instance : Object = module.new()
	assert(instance.has_method(base_method_name))
	var handle := WorldGeneratorHandle.new()
	handle.name = module_name
	handle._module_script = module
	handle._instance = instance
	handle._generator_function = Callable.create(instance, base_method_name)
	
	var ui_method_name := base_method_name + "_ui"
	if instance.has_method(ui_method_name):
		handle._generator_ui_function = Callable.create(instance, ui_method_name)
		
	return handle
	
