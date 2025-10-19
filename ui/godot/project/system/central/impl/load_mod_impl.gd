extends RefCounted

func is_mod_folder(folder: String) -> bool:
	return FileAccess.file_exists(folder.path_join('module.cfg'))
	
	
## Load module from given folder, returns its ConfigFile
func load_mod(folder: String) -> ConfigFile:
	print('Loading mod %s' % folder)
	var moddir := DirAccess.open(folder)
	if not moddir:
		push_error('Failed to open %s' % folder)
		return null
		
	var module_cfg_path := folder.path_join('module.cfg')
	if not FileAccess.file_exists(module_cfg_path):
		push_error('Failed to find module.cfg in %s' % folder)
		return null
	# load module info
	var modconfig := ConfigFile.new()
	var error := modconfig.load(module_cfg_path)
	if error != OK:
		push_error('Failed to load module %s file module.cfg, error is: %s' % [module_cfg_path, error_string(error)])
		return null
		
	var mod_name : String = modconfig.get_value("main", "module_name")
	
	if mod_name == "":
		push_error('Incorrect module.cfg, name can not be empty')
		return null
		
	# TODO: Read from modconfig, raise error if file not preset
	var data_path := folder.path_join('data.pck')
	if FileAccess.file_exists(data_path):
		if not ProjectSettings.load_resource_pack(data_path):
			push_error('Failed to load module %s data.pack' % [folder])
			
	# now, load module components. After we have loaded pack file,
	# we can now look for it in content folder

	
	if not load_mod_content(modconfig):
		return null
	
	print("success")
	return modconfig
	
## Returns all .gd scripts in target location
func load_mod_content_files(target_dir: String) -> Array[Resource]:
	var result: Array[Resource] = []
	var target_files := DirAccess.get_files_at(target_dir)
	for file : String in target_files:
		var full_file_path := target_dir.path_join(file)
		if file.get_extension() == "gd" or file.get_extension() == "gdc":
			var script : Resource = load(full_file_path)
			var test_instance : Object = script.new()
			if test_instance is not RefCounted:
				push_error('Failed to load %s as it does not inherit RefCounted' % [full_file_path])
				test_instance.free()
				continue
				
			result.append(script)
			
	return result
	
func load_mod_images(target_dir: String) -> Array[Resource]:
	var result: Array[Resource] = []
	var target_files := DirAccess.get_files_at(target_dir)
	for file : String in target_files:
		var full_file_path := target_dir.path_join(file)
		if file.get_extension() == "import":
			var resource := load(full_file_path.get_basename()) # remove .import extension
			
			if not resource:
				push_error("Failed to load %s" % [full_file_path])
				continue
			if not (resource is Image):
				push_error("Resource at %s is not an image" % [full_file_path])
				continue
			result.append(resource as Image)
	return result
	
func load_mod_gfx(target_dir: String) -> void:
	# load biomes gfx
	var biomes_dir_path := target_dir.path_join("biomes")
	if DirAccess.dir_exists_absolute(biomes_dir_path):
		var images := load_mod_images(biomes_dir_path)
		for image in images:
			var biome_name : StringName = image.resource_path.get_basename().get_file()
			if not (image is Image):
				print("%s is not an Image instance. Did you forget to set up import settings properly?" % [image.resource_path] )
			GfxRegistry.register_biome_image(biome_name, image as Image)
		
	
			
	
## Load content of the module. Looks for appropriate files in appropriate
## locations and registers them.
func load_mod_content(modconfig: ConfigFile) -> bool:
	var mod_name : String = modconfig.get_value("main", "module_name")
	var mod_info := ModInfo.new()
	mod_info.name = mod_name
	mod_info.config = modconfig
	
	var loaded_mod_path := "res://content".path_join(mod_name)
	
	mod_info.path = loaded_mod_path
	assert(mod_info.is_valid())
	
	# Check world gen folder
	var world_gen_folder_path := loaded_mod_path.path_join("worldgen")

	# DirAccess.dir_exists_absolute doesn't work here :(
	var mod_dir := DirAccess.open(loaded_mod_path)
	if mod_dir == null:
		return false
	
	if mod_dir.dir_exists("worldgen"):
		print("Loading wordlgen modules")
		# load every file in this folder
		var target_scripts := load_mod_content_files(world_gen_folder_path)
		for script: Script in target_scripts:
			WorldBuilderRegistry.register_module(mod_info, script)
	if mod_dir.dir_exists("gfx"):
		print("Loading gfx")
		load_mod_gfx(loaded_mod_path.path_join("gfx"))
		
	
	return true
	
func load_mods_from_folder(folder: String) -> Array:

	var dir := DirAccess.open(folder)
	var result := []
	
	if not dir:
		return result

	var moddirs := dir.get_directories()
		
	for moddir_name: String in moddirs:
		var full_mod_path := folder.path_join(moddir_name)
		
		if not is_mod_folder(full_mod_path):
			push_error('Folder %s is not a mod, skipping' % [full_mod_path])
			continue
			
		var modconfig := load_mod(full_mod_path)
		if modconfig == null:
			continue
			
		result.append(modconfig)
			
	return result
