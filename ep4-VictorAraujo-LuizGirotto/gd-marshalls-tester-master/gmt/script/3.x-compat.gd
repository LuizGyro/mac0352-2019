extends Node

func load_tester():
	get_tree().change_scene("res://scene/3.x/tester.tscn")

func load_listener():
	get_tree().change_scene("res://scene/3.x/listener.tscn")

func load_sender():
	get_tree().change_scene("res://scene/3.x/sender.tscn")

func set_dest_address(udp, ip, port):
	udp.set_dest_address(ip, port)

func set_physics_fps(fps):
	ProjectSettings.set_setting("physics/common/physics_fps", fps)

func enable_proc(node, enable):
	node.set_process(enable)

func enable_phys(node, enable):
	node.set_physics_process(enable)

func ByteArr(data=null):
	if data != null:
		return PoolByteArray(data)
	return PoolByteArray()

func IntArr(data=null):
	if data != null:
		return PoolIntArray(data)
	return PoolIntArray()

func RealArr(data=null):
	if data != null:
		return PoolRealArray(data)
	return PoolRealArray(data)

func StringArr(data=null):
	if data != null:
		return PoolStringArray(data)
	return PoolStringArray()

func Vec2Arr(data=null):
	if data != null:
		return PoolVector2Array(data)
	return PoolVector2Array()

func Vec3Arr(data=null):
	if data != null:
		return PoolVector3Array(data)
	return PoolVector3Array()

func ColorArr(data=null):
	if data != null:
		return PoolColorArray(data)
	return PoolColorArray()

func T2D(d1=null, d2=null):
	if d1 != null and d2 != null:
		return Transform2D(d1, d2)
	if d1 != null:
		return Transform2D(d1)
	return Transform2D()

func Bas(data=null):
	if data != null:
		return Basis(data)
	return Basis()