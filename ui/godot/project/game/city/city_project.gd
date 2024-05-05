extends RefCounted

## City project is a base class for constructibles in the city. It is something
## that requires workforce and may require some number of resources.
##
## An simplest example of the project is construction project - building an
## improvement. Another example is cutting down forest - there is no resources
## involed, but workforce is still required
## All projects should extend CityProject and must implement its methods, in
## addition to standard serialization methods

class_name CityProject

func take_resources(resources: Dictionary) -> void:
	assert(false, "Please implement this method")
	push_error("Please implement this method")
	
## Return true if project is finished. City will remove your project from queue
## and call appropriate callback.
func is_finished() -> bool:
	assert(false, "Please implement this method")
	push_error("Please implement this method")
	return false

## Invalid project, in in-game terms. For example, land where construction
## was done is now occupied by enemy, or is flooded. Impossible projects
## will not be given any resources and will be generally skipped in
## processing
func is_possible() -> bool:
	assert(false, "Please implement this method")
	push_error("Please implement this method")
	return false


func turns_without_progress() -> int:
	assert(false, "Please implement this method")
	push_error("Please implement this method")
	return 0
