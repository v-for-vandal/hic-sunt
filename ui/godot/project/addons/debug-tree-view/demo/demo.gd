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
