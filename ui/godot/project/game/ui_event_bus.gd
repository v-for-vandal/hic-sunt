extends Node

#class_name  GameUiEventBus

enum ActionType { PRIMARY, SECONDARY }
var _main_interaction
var _region_interaction
var _world_inetraction

class UIEvent:
	extends RefCounted
	var _accepted := false
	
	func accept() -> void:
		_accepted = true
	
	
class UIEventWithCoords:
	extends UIEvent
	var qr_coords: Vector2i
	var surface: GameTileSurface
	
class UIMovementEvent:
	extends UIEventWithCoords
	var prev_qr_coords: Vector2i
	
	func _to_string()-> String:
		return "movement event from %s to %s" % [prev_qr_coords, qr_coords]
	
class UIActionEvent:
	extends UIEventWithCoords
	
	var action_type : ActionType
	
	func _to_string()-> String:
		return "action event at %s of %s" % [ qr_coords, ActionType.find_key(action_type)]

class WorldUIActionEvent:
	extends UIActionEvent
	
	func _to_string()-> String:
		return "World: " + super() 
	
class RegionUIActionEvent:
	extends UIActionEvent
	
	func _to_string()-> String:
		return "World: " + super() 

class WorldUIMovementEvent:
	extends UIMovementEvent
	
	func _to_string()-> String:
		return "World: " + super() 
	
class RegionUIMovementEvent:
	extends UIMovementEvent
	
	func _to_string()-> String:
		return "World: " + super() 

	
class CancellationEvent:
	extends UIEvent

#signal world_cell_primary_action(qr_coords: Vector2i)
#signal region_cell_clicked(region_object: RegionObject, qr_coords: Vector2i, mouse_button_index: int)
#signal cancelled()

func _process_event( event )-> void:
	#print("Processing event: ", event)
	for consumer : Object in [_main_interaction, _region_interaction, _world_inetraction]:
		if consumer != null:
			#print('Consumer: ', consumer)
			if consumer is Node and consumer.get_parent() == null:
				# orphan. This code is for region/world map, because when one
				# is inactive, we remove it from the tree
				#print('\tskipped, orphan')
				continue
			consumer.on_ui_event(event)
			if event._accepted:
				#print('\tevent accepted, stopping')
				break
			
func emit_event(event)-> void:
	assert(event is UIEvent)
	
	_process_event(event)

func emit_cancellation()-> void:
	var event := CancellationEvent.new()
	_process_event(event)

func set_world_interaction(interaction : Object)-> void:
	# No need to send cancellation event here?
	# we expect that it will be set only once
	assert(_world_inetraction == null)
	_world_inetraction = interaction
	
func set_region_interaction(interaction)-> void:
	assert(_region_interaction == null)
	_region_interaction = interaction
	

func set_main_interaction(interaction) -> void:
	if _main_interaction != null:
		# notify previous interaction that it is cancelled
		var cancellation_event = CancellationEvent.new()
		_main_interaction.on_ui_event(cancellation_event)
	_main_interaction = interaction
	
func remove_main_interaction(interaction) -> void:
	if _main_interaction == interaction:
		_main_interaction = null

class UnitConsumer:
	func on_ui_event(event: UIEvent) -> bool:
		var as_world_event := event as WorldUIActionEvent
		if as_world_event != null and as_world_event.action_type == ActionType.SECONDARY:
			print("moving to world cell: ", as_world_event.qr_coords)
			return true 

		return false


# TODO: move to separate UserSettings class or Central clas
func mouse_button_to_action_type(mouse_button: MouseButton) -> ActionType:
	if mouse_button == MouseButton.MOUSE_BUTTON_LEFT:
		return ActionType.PRIMARY
	else:
		return ActionType.SECONDARY

