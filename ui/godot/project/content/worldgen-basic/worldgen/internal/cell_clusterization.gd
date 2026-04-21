extends RefCounted

class ClusterData:
	extends RefCounted

	## Neigbouring cells to this cluster. May belong to other clusters
	var neighbours: Dictionary[int, int]

## Random number generator used, mostly, for seeding. This is public variable,
## you can replace generator with your own
var rng := RandomNumberGenerator.new()

var _voronoi: Voronoinator
var _cell_to_cluster := PackedInt32Array()

var _clusters: Dictionary[int, ClusterData]

const _MAX_ATTEMPTS = 10000


## Creates clusterization
## voronoi - cells to clusterize
func _init(voronoi: Voronoinator) -> void:
	_voronoi = voronoi
	_cell_to_cluster.resize(_voronoi.voronoi_cells.size())
	_cell_to_cluster.fill(0)

## Seeds `count` new clusters, starting from `start_idx`. Returns list of
## created clusters
## Optional `forbid_neighbours` will prevent new clusters from spawning
## in the perimeter of existing cluster. That is new cluster will not touch
## any of the existing clusters (including created by this function itself)
func seed_n_clusters(count: int, start_idx: int = 1, forbid_neighbours : bool = false) -> Array[int]:
	if start_idx <= 0:
		push_error("Start idx must be >= 1, but it is %s" % start_idx)
		start_idx = 1
		
	var count_seeded := 0
	var next_cluster_id := start_idx
	var result : Array[int] = []
	
	for _z in range(_MAX_ATTEMPTS): # prevent infinite cycle
		if count_seeded == count :
			break

		var cluster_start := rng.randi_range(0, _voronoi.voronoi_cells.size() - 1)
		if cell_cluster(cluster_start) != 0:
			# already taken
			continue
		if forbid_neighbours and is_neighbour_to_any_cluster(cluster_start):
			# dont allow tectonic plates to touch each other
			continue

		# ok, good cluster start. Find first available cluster id
		while next_cluster_id in _clusters:
			next_cluster_id += 1
			
		var this_cluster_id := next_cluster_id
		add_to_cluster(cluster_start, this_cluster_id)
		result.append(this_cluster_id)
		next_cluster_id += 1
		count_seeded += 1
		
	return result

func add_to_cluster(cell_id: int, cluster_id: int) -> void:
	if cluster_id != 0 and cluster_id not in _clusters:
		_clusters[cluster_id] = ClusterData.new()

	var cluster_data := _clusters[cluster_id]
	# for now, forbid changing cluster of a cell
	var current_cell_cluster := _cell_to_cluster[cell_id]
	if current_cell_cluster != 0 and current_cell_cluster != cluster_id:
		push_error("Moving cell between clusters is forbidden %d -> %d" % [current_cell_cluster, cluster_id])
		return

	_cell_to_cluster[cell_id] = cluster_id
	# update neighbours
	var neighbours := _voronoi.neighboring_cells(cell_id)
	for n in neighbours:
		if _cell_to_cluster[n] != cluster_id:
			cluster_data.neighbours[n] = cluster_data.neighbours.get_or_add(n, 0) + 1
			
	# Remove self from neighbors of self-cluster.
	cluster_data.neighbours.erase(cell_id)


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


## Returns cluster for this cell_id or 0 if it is not in cluster
func cell_cluster(cell_id: int) -> int:
	return _cell_to_cluster[cell_id]

## Returns true if this cell is in any cluster, false otherwise
func is_cell_in_any_cluster(cell_id: int) -> bool:
	return cell_cluster(cell_id) != 0

## Number of clusters. Please note that
## 1. cluster ids start with 1 (not 0)
## 2. cluster ids don't have to start at 1 and may have gaps. E.g. [2, 5, 12]
## Ergo, you can't use clusters_count() for iteration
func clusters_count() -> int:
	return _clusters.size()
	
## Get ids of all clusters in clusterization
func get_cluster_ids() -> Array[int]:
	return _clusters.keys()
	
## Returns true if such cluster exists
## Always return true for cluster_id == 0
func has_cluster_with_id(cluster_id: int) -> bool:
	if cluster_id == 0:
		return true
	return _clusters.has(cluster_id)


## Returns all neighbours for given cluster
func cluster_neighbours(cluster_id: int) -> Array[int]:
	if cluster_id not in _clusters:
		push_error("Unknown cluster %d" % cluster_id)
		return []
	return _clusters[cluster_id].neighbours.keys()
