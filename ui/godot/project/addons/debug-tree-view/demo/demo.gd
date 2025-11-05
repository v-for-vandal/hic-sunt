extends Control

func _ready() -> void:
	var control : DebugTree.TreeControl = $DebugTree.get_debug_control()
	var group_control := control.add_group("parent/child")
	group_control.add_text_node("text1", "Lopus igmus and so on")
	control.add_text_node("text2", "Some other complicated text")
