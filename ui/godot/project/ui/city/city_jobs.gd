extends Control

var _region_object: RegionObject

func _clear() -> void:
	if _region_object == null:
		return
	_clear_widgets()
	_disconnect_region_object(_region_object)
	
	
func _clear_widgets() -> void:
	var children = $%JobsView.get_children()
	for child in children:
		$%JobsView.remove_child(child)
		child.queue_free()
		
		
func _add_job_widget(job_id: String, job_count: int):
	var name_label = RichTextLabel.new()
	name_label.autowrap_mode =TextServer.AUTOWRAP_OFF
	name_label.fit_content = true
	name_label.bbcode_enabled = true
	name_label.scroll_active = false
	name_label.text = job_id
	$%JobsView.add_child(name_label)
	
	var value_label = Label.new()
	value_label.set_text(str(job_count))
	$%JobsView.add_child(value_label)
	
	
func _set_jobs(jobs: Dictionary[StringName, int]) -> void:
	for job_id : StringName in jobs:
		_add_job_widget(job_id, jobs[job_id])
		
func load_region(region: RegionObject) -> void:
	assert(region != null, "Initializing with empty region")
	_clear()
	
	_region_object = region
	_update_region()
	_connect_region_object(_region_object)
	
func _update_region() -> void:
	_clear_widgets()
	var city_id_opt := _region_object.get_city_id()
	if city_id_opt.is_empty():
		return
		
	# TODO: This will not permit displaying cities of other civs
	var city : City = CurrentGame.get_current_player_civ().find_city_by_id(city_id_opt)
	
	if city == null:
		push_error("Can't fetch city with id: ", city_id_opt)
		return
		
	_set_jobs(_region_object.get_jobs(CurrentGame.get_current_player_ruleset()))
	
func _on_region_changed(area: Rect2i, flags: int) -> void:
	_update_region()
	
func _connect_region_object(region_object: RegionObject) -> void:
	region_object.region_changed.connect(_on_region_changed)
	
func _disconnect_region_object(region_object: RegionObject) -> void:
	region_object.region_changed.disconnect(_on_region_changed)
	
