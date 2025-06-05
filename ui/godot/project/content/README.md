## Into

All user and system mods/dlc/content are loaded here on startup

Module structure is as follows:
-- root
 -- module.cfg
	// basic description of this module
 -- worldgen/
	// if present, we will try to load world generation
	// module from here 
	// Every script on top level that has get_module_info method
	//  will be loaded as module
	// For clarity, we recomend moving internal scripts into some
	// subfolder
