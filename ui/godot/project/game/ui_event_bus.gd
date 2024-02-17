extends Node

#class_name  GameUiEventBus

enum ActionType { PRIMARY, SECONDARY }
var _consumers: Array = []

class UIEvent:
	extends RefCounted
	var _accepted := false
	
	func accept() -> void:
		_accepted = true
	
	
class UIEventWithCoords:
	extends UIEvent
	var qr_coords: Vector2i
	var surface
	
class UIMovementEvent:
	extends UIEventWithCoords
	var prev_qr_coords: Vector2i
	
	func _to_string()-> String:
		return "movement event from %s to %s" % [prev_qr_coords, qr_coords]
	
class UIActionEvent:
	extends UIEventWithCoords
	
	var action_type : ActionType
	
	func _to_string()-> String:
		return "action event at %s of %s" % [ qr_coords, action_type]

class WorldUIActionEvent:
	extends UIActionEvent
	
class RegionUIActionEvent:
	extends UIActionEvent

class WorldUIMovementEvent:
	extends UIMovementEvent
	
class RegionUIMovementEvent:
	extends UIMovementEvent

	
class CancellationEvent:
	extends UIEvent

#signal world_cell_primary_action(qr_coords: Vector2i)
#signal region_cell_clicked(region_object: RegionObject, qr_coords: Vector2i, mouse_button_index: int)
#signal cancelled()

func _process_event( event ):
	print("Processing event: ", event)
	for consumer in _consumers:
		consumer.on_ui_event(event)
		if event._accepted:
			break
			
func emit_event(event):
	assert(event is UIEvent)
	
	_process_event(event)

func emit_cancellation():
	var event := CancellationEvent.new()
	_process_event(event)


func add_consumer(consumer)-> void:
	_consumers.insert(0, consumer)

class UnitConsumer:
	func on_ui_event(event: UIEvent) -> bool:
		var as_world_event = event as WorldUIActionEvent
		if as_world_event != null and as_world_event.action_type == ActionType.SECONDARY:
			print("moving to world cell: ", as_world_event.qr_coords)
			return true 

		return false
		
class BuildSiteConsumer:
	func on_region_cell_selected(region_object: RegionObject, qr_coords: Vector2i):
		print("Building at: ", qr_coords)

# TODO: move to separate UserSettings class or Central clas
func mouse_button_to_action_type(mouse_button: MouseButton) -> ActionType:
	if mouse_button == MouseButton.MOUSE_BUTTON_LEFT:
		return ActionType.PRIMARY
	else:
		return ActionType.SECONDARY

