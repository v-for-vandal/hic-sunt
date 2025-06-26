extends RefCounted

class_name ModInfo

## Name of the module
var name : String
## Relative path to this module
var path : String
## Main module configuration file
var config: ConfigFile

func is_valid() -> bool:
	if len(name) == 0:
		return false
	if len(path) == 0:
		return false
		
	return true
