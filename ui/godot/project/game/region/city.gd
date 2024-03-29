extends RefCounted

class_name City

var _region_id : String
var _city_id : String
var _player : int = 0
var _city_finance : Dictionary = {}
var _territory: Dictionary = {} # dict [region_id -> whatever]
var _build_queue : Array

var _serializable_properties: Array[StringName] = [
	"_region_id",
	"_city_id",
	"_player",
	"_city_finance",
	"_territory",
	"_build_queue"
	]

func get_city_id() -> String:
	return _city_id
	
func get_region_id() -> String:
	return _region_id
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

func add_to_build(improvement_id: String, region: RegionObject, region_qr: Vector2i) -> void:
	# TODO: Restore. for now, just set improvement on tile
	#_build_queue.append(BuildSite.CreateBuildSite(improvement_id, region, region_qr))
	region.set_improvement(region_qr, improvement_id)
	# notify about updates
	GlobalSignalsBus.emit_region_cell_changed(region.get_region_id(), region_qr)


	
func next_turn() -> void:
	# Collect profit and loss from region
	var profit_and_loss := {}
	for world_qr in _territory:
		ResourceEconomyLibrary.update(profit_and_loss,
			CurrentGame.current_world.get_region(world_qr).collect_profit_and_loss(
				CurrentGame.get_ruleset()))
	
	# aggregate them
	var total : Dictionary = ResourceEconomyLibrary.combine(profit_and_loss.profit, profit_and_loss.loss)
	
	# Finance build queue
	if not _build_queue.is_empty():
		var current_buildsite = _build_queue[0].add_resources(total)
		if current_buildsite.is_ready():
			# TODO: Support building in other regions
			#CurrentGame.current_world.get_region_by_id(_region_id).set_improvement(
			#		current_buildsite.improvement_id(),
			#		)
			_build_queue.pop_front()
		
	# Add to the city finance
	var new_finance := ResourceEconomyLibrary.combine(_city_finance, total)
	_city_finance = new_finance
	
func get_serializable_properties() -> Array[StringName]:
	return _serializable_properties

func serialize_to_variant() -> Dictionary:
	return SerializeLibrary.serialize_to_variant(self)
	
func parse_from_variant(data : Dictionary) -> void:
	SerializeLibrary.parse_from_variant(self, data)
