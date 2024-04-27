extends Node

'''
This file contains all of the needed utility functions that are used in pluginMenu.gd .
center_polygon:
	Description:
		Centers a polygon at 0,0.
		This is ported from a python script generated by Google Bard
	Args:
		polygon - the polygon that will be centered at 0,0
	Returns:
		A list of points that define the centered polygon.
'''
func center_polygon(polygon : Array, center : Vector2) -> PackedVector2Array:
	var centered_polygon := PackedVector2Array()
	for point in polygon:
		var centered_point = Vector2(point[0] - center[0], point[1] - center[1])
		centered_polygon.append(centered_point)
	
	return centered_polygon
