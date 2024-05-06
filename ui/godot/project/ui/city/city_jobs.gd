extends Control


func _clear() -> void:
	var children = $%JobsView.get_children()
	for child in children:
		$%JobsView.remove_child(child)
		child.queue_free()
		
		
func _add_job_widget(job_id: String, job_count: int):
	var name_label = RichTextLabel.new()
	name_label.autowrap_mode =TextServer.AUTOWRAP_OFF
	name_label.fit_content = false
	$%JobsView.add_child(name_label)
	
	var value_label = Label.new()
	value_label.set_text(str(job_count))
	$%JobsView.add_child(value_label)
	
	
func _set_jobs(jobs: Dictionary) -> void:
	for job_id in jobs:
		_add_job_widget(job_id, jobs[job_id])
		
func load_region(region: RegionObject) -> void:
	_clear()
	var city_id_opt := region.get_city_id()
	if city_id_opt.is_empty():
		return
		
	# TODO: This will not permit displaying cities of other civs
	var city : City = CurrentGame.get_current_player_civ().find_city_by_id(city_id_opt)
	
	if city == null:
		push_error("Can't fetch city with id: ", city_id_opt)
		return
		
	_set_jobs(region.get_jobs(CurrentGame.get_current_player_ruleset()))
