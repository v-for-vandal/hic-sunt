extends Control

func _ready() -> void:
	var control: DebugTree.TreeControl = $DebugTree.get_debug_control()
	var group_control := control.add_group("parent/child")
	group_control.add_text_node("text1", "Lopus igmus and so on")
	control.add_text_node("text2", "Some other complicated text")

	var polygon := Polygon2D.new()
	polygon.polygon = PackedVector2Array([Vector2(0.0, 0.0), Vector2(100.0, 0.0), Vector2(100.0, 100.0), Vector2(0.0, 100.0)])
	polygon.color = Color.RED
	control.add_2d_node("polygon", polygon)
	
	var image := load("res://addons/debug-tree-view/demo/icon.svg")
	control.add_image_node("image", image)
	
	# add overlay for image. Useful to highlight some part of the image.
	var image_node_with_overlay := control.add_image_node("image_with_overlay", image)
	# create_rect_overlay will create simple overlay that does not mess up with image node
	# functionality (that is, mouse filter, focus mode and so on will be propertly set up)
	image_node_with_overlay.add_to_overlay(DebugTree.create_rect_overlay(
		Rect2i(Vector2i.ZERO, image.get_size() / 2),
		Color.ORANGE,
		"test_overlay"
	))
	image_node_with_overlay.add_to_overlay(DebugTree.create_rect_overlay(
		Rect2i(image.get_size() / 2, image.get_size() / 2),
		Color.ORANGE,
		"test_overlay2"
	))
