extends HBoxContainer

var _seed : int = 0

func _ready() -> void:
	_gen_new_seed()

func _on_button_pressed() -> void:
	_gen_new_seed()

func _gen_new_seed() -> void:
	$Seed.text = str(randi())
	
func get_seed() -> int:
	return int($Seed.text)
	
