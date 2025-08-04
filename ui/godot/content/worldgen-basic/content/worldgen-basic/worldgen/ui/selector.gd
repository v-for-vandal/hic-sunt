extends HBoxContainer


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	# get all possible heightmaps
	var available_generators := WorldBuilderRegistry.get_modules_for_category(WorldBuilderRegistry.CATEGORY.Heightmap)
	
	for generator_info in available_generators:
		var name := generator_info.name
		
		if generator.has_method("get_ui"):
			var ui_element = generator.get_ui()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
