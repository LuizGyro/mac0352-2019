extends Node

func load_tester():
	get_tree().change_scene("res://scene/2.1/tester.tscn")

func load_listener():
	get_tree().change_scene("res://scene/2.1/listener.tscn")

func load_sender():
	get_tree().change_scene("res://scene/2.1/sender.tscn")

func set_dest_address(udp, ip, port):
	udp.set_send_address(ip, port)

func set_physics_fps(fps):
	pass

func enable_proc(node, enable):
	node.set_process(enable)

func enable_phys(node, enable):
	node.set_fixed_process(enable)

func ByteArr(data=null):
	if data != null:
		return RawArray(data)
	return RawArray()

func IntArr(data=null):
	if data != null:
		return IntArray(data)
	return IntArray()

func RealArr(data=null):
	if data != null:
		return FloatArray(data)
	return FloatArray(data)

func StringArr(data=null):
	if data != null:
		return StringArray(data)
	return StringArray()

func Vec2Arr(data=null):
	if data != null:
		return Vector2Array(data)
	return Vector2Array()

func Vec3Arr(data=null):
	if data != null:
		return Vector3Array(data)
	return Vector3Array()

func ColorArr(data=null):
	if data != null:
		return ColorArray(data)
	return ColorArray()

func T2D(d1=null, d2=null):
	if d1 != null and d2 != null:
		return Matrix32(d1, d2)
	if d1 != null:
		return Matrix32(d1)
	return Matrix32()

func Bas(data=null):
	if data != null:
		return Matrix3(data)
	return Matrix3()