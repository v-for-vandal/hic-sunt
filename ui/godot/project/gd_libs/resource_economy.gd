extends RefCounted

class_name ResourceEconomyLibrary

static func combine(profit: Dictionary, loss: Dictionary) -> Dictionary:
	var result := profit.duplicate()
	update(result, loss)
	
	return result
	
static func update(target: Dictionary, input: Dictionary) -> void:
	for res_id : String in input:
		target[res_id] = target.get(res_id, 0) + input.get(res_id, 0)
		
static func multiply(target: Dictionary, multiplier: int) -> void:
	for res_id : String in target:
		target[res_id] = target[res_id] * multiplier
			
	
