extends GridContainer


# Called when the node enters the scene tree for the first time.
func get_config() -> Dictionary:
	return {
		'min-height' : $MinHeightSlider.value,
		'max-height' : $MaxHeightSlider.value
	}
