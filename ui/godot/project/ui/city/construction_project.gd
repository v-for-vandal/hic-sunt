extends HBoxContainer

func setup(project: ConstructionProject) -> void:
	var improvement_id : String = project.get_improvement_id()
	$%Name.text = improvement_id
