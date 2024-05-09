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

## Notify that project has changed. This signal is emited automatically
## by the system, there is no need to call it manually
signal changed()

## Take resources that the project need. Modify the 'resources' accordingly
## Return true if the was any change, false otherwrise. Doesn't matter what
## kind of chnage - return value is used to notify UI so that it can update
## itself
func take_resources(resources: Dictionary) -> bool:
	assert(false, "Please implement this method")
	push_error("Please implement this method")
	return false
	
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

## This method must return dictionary with 3 members:
## 1. progress : int [0..100] - approximation of current progress
## 2. turns_left : int - approximately how many turns left. Negative value
##    means infinity. 0 has no special meaning 
## 3. turns_without_progress - how many turns without progress
## TODO: DO we need turns_without_progress on 
func progress_estimate() -> Dictionary:
	assert(false, "Please implement this method")
	push_error("Please implement this method")
	return {}
	
func turns_without_progress() -> int:
	assert(false, "Please implement this method")
	push_error("Please implement this method")
	return 0

