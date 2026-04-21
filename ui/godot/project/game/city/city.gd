extends RefCounted

class_name City

# signals

# private variables
var _region_id: String
var _city_id: String
var _player: int = 0
var _city_finance: Dictionary = { }
var _territory: Dictionary = { } # dict [region_id -> whatever]
var _projects_queue := CityProjectsQueue.new()

var _serializable_properties: Array[StringName] = [
	"_region_id",
	"_city_id",
	"_player",
	"_city_finance",
	"_territory",
	"_projects_queue",
]


func get_city_id() -> String:
	return _city_id


func get_region_id() -> String:
	return _region_id


func get_region_object() -> RegionObject:
	return CurrentGame.get_current_world().get_region_by_id(_region_id)


# This constructor is used by civilization.gd. It is not supposed to be used
# directly
static func create_new_city(city_id: String, region_id: String) -> City:
	assert(!city_id.is_empty())
	assert(!region_id.is_empty())
	var result := City.new()
	result._region_id = region_id
	result._territory[region_id] = true
	result._city_id = city_id
	# TODO: Update region object itself
	return result

	#func add_to_build(improvement_id: String, region: RegionObject, region_qr: Vector2i) -> void:
	## TODO: Restore. for now, just set improvement on tile
	##_build_queue.append(BuildSite.CreateBuildSite(improvement_id, region, region_qr))
	#region.set_improvement(region_qr, improvement_id)
	## notify about updates
	#GlobalSignalsBus.emit_region_cell_changed(region.get_region_id(), region_qr)


func add_project(project: CityProject) -> void:
	assert(project != null, "project must not be null")
	_projects_queue.add_project(project, _projects_queue.size())


func get_projects_queue() -> CityProjectsQueue:
	return _projects_queue


func build_pnl() -> Dictionary:
	# TODO: This function must use cache, because it is computationally
	# expensive
	var profit: Dictionary = { }
	var losses: Dictionary = { }

	var ruleset := CurrentGame.get_current_player_ruleset()
	# this is list of jobs
	var region_jobs: Dictionary = get_region_object().get_jobs(ruleset)
	print("region jobs: ", region_jobs)
	for job_id: String in region_jobs:
		var job_count: int = region_jobs[job_id] # for now, assume that every job is occupied
		var job_info := ruleset.get_job_info(job_id)
		var job_profit: Dictionary = job_info.output
		var job_losses: Dictionary = job_info.input
		#print("Job id: ", job_id)
		#print("Job profit: ", job_profit)
		#print("Job losses: ", job_losses)
		#print("Job count: ", job_count)
		ResourceEconomyLibrary.multiply(job_profit, job_count)
		ResourceEconomyLibrary.multiply(job_losses, job_count)
		# multiply everything by count

		ResourceEconomyLibrary.update(profit, job_profit)
		ResourceEconomyLibrary.update(losses, job_losses)

	var result := {
		"profit": profit,
		"losses": losses,
		"total": ResourceEconomyLibrary.combine(profit, losses),
	}

	return result


func next_turn() -> void:
	# Collect profit and loss from region
	var total: Dictionary = build_pnl().total

	# Finance projects queue
	_projects_queue.process(total)

	# Add to the city finance
	var new_finance := ResourceEconomyLibrary.combine(_city_finance, total)
	_city_finance = new_finance


# This is internal method, we use it to clear object before parsing
func _clear() -> void:
	_region_id = ""
	_city_id = ""
	_player = 0
	_city_finance = { }
	_territory = { } # dict [region_id -> whatever]
	_projects_queue = CityProjectsQueue.new()


func get_serializable_properties() -> Array[StringName]:
	return _serializable_properties


func serialize_to_variant() -> Dictionary:
	return SerializeLibrary.serialize_to_variant(self)


func parse_from_variant(data: Dictionary) -> void:
	_clear()
	SerializeLibrary.parse_from_variant(self, data)
