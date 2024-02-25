extends Camera2D

var zoom_factor := Vector2(0.5, 0.5)
var zoom_speed := 0.3
@onready var zoom_min := Vector2(0.4, 0.4) * zoom
@onready var zoom_max := Vector2(3.6, 3.6) * zoom
var cell_size := 100

var _zoom_tween : Tween

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
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
	
func _unhandled_input(event : InputEvent) -> void:
	
	var change_zoom := false
	var new_zoom: Vector2
	if event.is_action_pressed("ui_zoom_in"):
		change_zoom = true
		new_zoom = zoom + zoom_factor
	elif  event.is_action_pressed("ui_zoom_out"):
		change_zoom = true
		new_zoom = zoom - zoom_factor
		
	if change_zoom:
		new_zoom = new_zoom.clamp(zoom_min, zoom_max)
		if _zoom_tween:
			_zoom_tween.kill()
			
		print("zooming  to", new_zoom)
		_zoom_tween = create_tween()
		_zoom_tween.tween_property(self, "zoom", new_zoom, zoom_speed)
