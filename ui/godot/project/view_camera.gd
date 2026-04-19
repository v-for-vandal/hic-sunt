extends Camera2D

#var zoom_factor := Vector2(0.5, 0.5)
var zoom_speed := 0.3
@onready var zoom_min := Vector2(0.2, 0.2) * zoom
@onready var zoom_max := Vector2(3.6, 3.6) * zoom

const MIN_ZOOM_FACTOR := Vector2(0.7, 0.7)
const ZOOM_PERCENTAGE := 0.15
const ZOOM_SPEED := 0.3

var cell_size := 100

var _zoom_tween : Tween

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	zoom = zoom.clamp(zoom_min, zoom_max)
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	var displacement: Vector2 = Vector2.ZERO
	if Input.is_action_pressed("ui_up"):
		displacement += Vector2.UP
	if Input.is_action_pressed("ui_down"):
		displacement += Vector2.DOWN
	if Input.is_action_pressed("ui_right"):
		displacement += Vector2.RIGHT
	if Input.is_action_pressed("ui_left"):
		displacement += Vector2.LEFT
		
	position += displacement * cell_size * delta / zoom




func _input(event: InputEvent) -> void:
	var handled := false

	var change_zoom := false
	var new_zoom: Vector2
	var zoom_factor: = Vector2(
		max(MIN_ZOOM_FACTOR.x, zoom.x * ZOOM_PERCENTAGE),
		max(MIN_ZOOM_FACTOR.y, zoom.y * ZOOM_PERCENTAGE),
	)

	if event.is_action_pressed("ui_zoom_in"):
		if zoom != zoom_max:
			change_zoom = true
			new_zoom = zoom + zoom_factor
			new_zoom = new_zoom.clamp(zoom_min, zoom_max)
	elif event.is_action_pressed("ui_zoom_out"):
		if zoom != zoom_min:
			change_zoom = true
			new_zoom =  zoom - zoom_factor
			new_zoom = new_zoom.clamp(zoom_min, zoom_max)


	if change_zoom:
		print("new zoom is: %s" % new_zoom)

		if _zoom_tween:
			_zoom_tween.kill()

		_zoom_tween = create_tween()
		_zoom_tween.tween_property(self, "zoom", new_zoom, ZOOM_SPEED)
		handled = true

	if event is InputEventMouseMotion and Input.is_mouse_button_pressed(MOUSE_BUTTON_LEFT):
		var move_event := event as InputEventMouseMotion
		if not move_event.relative.is_zero_approx():
			# invert displacement
			self.position -= move_event.relative / zoom
		handled = true

	if handled:
		get_viewport().set_input_as_handled()
		
#func _unhandled_input(event : InputEvent) -> void:
	#
	#var change_zoom := false
	#var new_zoom: Vector2
	#if event.is_action_pressed("ui_zoom_in"):
		#change_zoom = true
		#new_zoom = zoom + zoom_factor
	#elif  event.is_action_pressed("ui_zoom_out"):
		#change_zoom = true
		#new_zoom = zoom - zoom_factor
		#
	#if change_zoom:
		#new_zoom = new_zoom.clamp(zoom_min, zoom_max)
		#if _zoom_tween:
			#_zoom_tween.kill()
			#
		#_zoom_tween = create_tween()
		#_zoom_tween.tween_property(self, "zoom", new_zoom, zoom_speed)
