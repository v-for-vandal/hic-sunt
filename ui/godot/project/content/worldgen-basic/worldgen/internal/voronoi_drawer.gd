extends Node2D

var _voronoi: Voronoinator
var _colors: PackedColorArray

signal finished_drawing

func _init(voronoi: Voronoinator, colors: PackedColorArray) -> void:
	assert(voronoi.voronoi_cells.size() == colors.size())
	_voronoi = voronoi
	_colors = colors
	
func _draw()->void:
	for i in range(_voronoi.voronoi_cells.size()):
		var cell := _voronoi.voronoi_cells[i]
		var color := _colors[i]
		draw_colored_polygon(cell, color)
	finished_drawing.emit()
	
