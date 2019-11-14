extends Node

const Variator = preload("res://script/variator.gd")

class Obj extends Object:

	func _init():
		print("Object inited")

var obj = Label.new()

func test(good, path, p_seed=null):
	var vec2 = Vector2(-0.2, 27.3456)
	var vec3 = Vector3( pow(2, 21) +231.552, -32.3456, 53.2)
	var quat = Quat(.3, -.1, 24.3, 25.3)
	var data = [
		"漢字 UTF8 String 汉字",
		true,
		42,
		pow(2, 32)+42,
		-0.2,
		-2^33,
		vec2,
		Rect2(vec2, vec2.normalized().rotated((PI))),
		vec3,
		COMPAT.T2D(0.5, vec2),
		Plane(vec3, -.2),
		quat,
		AABB(vec3, vec3),
		COMPAT.Bas(quat),
		Transform(quat),
		Color(0.4, 0.5, .2, .1),
		path,
		obj,
		{
		},
		{
			"A key": "a value",
			"漢字 UTF8 String 汉字": 27,
			"sub": {
				"sub": {
					
				}
			}
		},
		[],
		["漢字 UTF8 String 汉字", vec3, vec2]
	]
	var all = []
	for d in data:
		var variator = Variator.new()
		variator.set_data(var2bytes(d), p_seed)
		variator.fuzzy_me()
		for d in variator.get_variations():
			all.append(d)
	for d in data:
		good.append(var2bytes(d))
	return all
