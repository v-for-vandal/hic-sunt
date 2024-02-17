extends RefCounted

class_name ResourceEconomyLibrary

static func combine(profit: Dictionary, loss: Dictionary):
	var result := profit.duplicate()
	update(result, loss)
	
	return result
	
static func update(target: Dictionary, input: Dictionary):
	for res_id in input:
		target[res_id] = target[res_id] + input.get(res_id, 0)
	
	for res_id in input:
		if not res_id in target:
			target[res_id] = target[res_id]
	
