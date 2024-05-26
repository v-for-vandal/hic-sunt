extends HBoxContainer

var _project : ConstructionProject

func setup(project: ConstructionProject) -> void:
	assert(_project == null, "No reusing of project widget is allowed")
	_project = project
	var improvement_id : String = project.get_improvement_id()
	$%Name.text = improvement_id
	project.changed.connect(_on_update_progress)
	
func _on_update_progress():
	var estimate := _project.progress_estimate()
	$%ProgressBar.value = estimate.progress
	
