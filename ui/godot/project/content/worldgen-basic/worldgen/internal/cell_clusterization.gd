extends RefCounted

class ClusterData:
	extends RefCounted

	## Neigbouring cells to this cluster. May belong to other clusters
	var neighbours: Dictionary[int, int]


var _voronoi: Voronoinator
var _cell_to_cluster := PackedInt32Array()

var _clusters: Dictionary[int, ClusterData]


func _init(voronoi: Voronoinator) -> void:
	_voronoi = voronoi
	_cell_to_cluster.resize(_voronoi.voronoi_cells.size())


func add_to_cluster(cell_id: int, cluster_id: int) -> void:
	if cluster_id != 0 and cluster_id not in _clusters:
		_clusters[cluster_id] = ClusterData.new()

	var cluster_data := _clusters[cluster_id]
	# for now, forbid changing cluster of a cell
	if _cell_to_cluster[cell_id] != 0:
		push_error("Moving cell between clusters is forbidden (for now0)")
		return

	_cell_to_cluster[cell_id] = cluster_id
	# update neighbours
	var neighbours := _voronoi.neighboring_cells(cell_id)
	for n in neighbours:
		if _cell_to_cluster[n] != cluster_id:
			cluster_data.neighbours[n] = cluster_data.neighbours.get_or_add(n, 0) + 1


## Checks that this node is neighbour to any cluster currently present.
## Optional paramter except allows you to skip checking one cluster.
## This method does not check that cell_id is in cluster, it only checks
## for being a neigbhour
func is_neighbour_to_any_cluster(cell_id: int, except: Array[int] = []) -> bool:
	for cluster_id in _clusters:
		if cluster_id in except:
			continue
		if cell_id in _clusters[cluster_id].neighbours:
			return true
	return false


func cell_cluster(cell_id: int) -> int:
	return _cell_to_cluster[cell_id]


func clusters_count() -> int:
	return _clusters.size()


func cluster_neighbours(cluster_id: int) -> Array[int]:
	if cluster_id not in _clusters:
		push_error("Unknown cluster %d" % cluster_id)
		return []
	return _clusters[cluster_id].neighbours.keys()
