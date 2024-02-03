extends HicSuntSystem

func _ready():
	var core_ruleset_path = ProjectSettings.globalize_path('res://gamedata/v1.0')
	var load_result = load_ruleset(core_ruleset_path)
	if load_result.success:
		print("Successfully loaded core ruleset: ", load_result.success)
	else:
		print("While loading core ruleset, there were errors: ", load_result.errors)
