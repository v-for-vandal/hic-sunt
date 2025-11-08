extends HBoxContainer

signal move_up(idx: int)
signal move_down(idx: int)

var _construction_project_scene := preload("res://ui/city/construction_project.tscn")


func setup(project: CityProject) -> void:
	if project is ConstructionProject:
		var construction_project_widget := _construction_project_scene.instantiate()
		construction_project_widget.setup(project)
		$%PlaceholderContainer.add_child(construction_project_widget)
	else:
		assert(false, 'Custom projects are not yet supported')
		push_error('Custom projects are not yet supported')


func _on_up_button_pressed() -> void:
	move_up.emit(get_index())


func _on_down_button_pressed() -> void:
	move_down.emit(get_index())
