extends Node2D

var basic_texture: Texture = load("res://resources/green512x512.png")
var highlight_texture: Texture = load("res://resources/red512x512.png")


func _ready() -> void:
	var parent := self.get_parent()
	if parent != null and parent is TileMapLayer:
		print("I am cell %s", parent.local_to_map(self.position))


func _on_area_2d_mouse_entered() -> void:
	print("Mouse entered at %s" % [self])
	$Polygon2D.texture = highlight_texture
	var parent := self.get_parent()
	if parent != null and parent is TileMapLayer:
		print("I am cell %s", parent.local_to_map(self.position))


func _on_area_2d_mouse_exited() -> void:
	$Polygon2D.texture = basic_texture
	print("Mouse exited at %s" % [self])
