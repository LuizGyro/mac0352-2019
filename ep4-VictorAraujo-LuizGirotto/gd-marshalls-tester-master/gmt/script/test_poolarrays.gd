extends Reference

const Variator = preload("res://script/variator.gd")

func test(good, p_seed=null):
	var bytes = COMPAT.ByteArr([1,2,3,4,5,6,11,12,13,14,55,255,123])

	var strings = COMPAT.StringArr([
		"A string",
		"",
		"A very long Sintrg dsrftd fdsf dsf sdf ",
		"An 漢字 UTF-8 string 汉字"
	])
	var vec2s = COMPAT.Vec2Arr([
		Vector2(-15, 23),
		Vector2(-35, 27),
		Vector2(256.3, -1)
	])
	var vec3s = COMPAT.Vec3Arr([
		Vector3(45, 654.3, PI),
		Vector3(45, PI, -2),
		Vector3(-PI, -54, 23.5)
	])
	var colors = COMPAT.ColorArr([
		Color(.2, .3, .5, .1),
		Color(.2, .3, .5),
		Color(1, .3, .5, .7)
	])
	var ints = COMPAT.IntArr([
		15, 27, 25035345, 456456341, 23546576474, 3466578
	])
	var reals = COMPAT.RealArr([
		12.3, .5435, 34545.4, .535645, 23423545.5, 34.5
	])
	var arrays = [bytes, strings, vec3s, colors, ints, reals]
	
	var all_variations = []
	var variator = Variator.new()
	for a in arrays:
		variator.set_data(var2bytes(a), p_seed)
		variator.fuzzy_me()
#		variator.variate_byte(7, 128)
#		variator.variate_byte_limit(7, 0, 255)
#		variator.variate_byte_rnd(7, 3)
#		variator.variate_byte_rnd(7, 5)
		for d in variator.get_variations():
			all_variations.append(d)

#	for s in strings:
#		variator.set_data(var2bytes(s))
#		variator.variate_byte(8+31, 128)
#		for i in range(0, 3):
#			variator.variate_byte_limit(8+30+i, 0, 255)
#			variator.variate_byte_rnd(8+30+i, 3)
#			variator.variate_byte_rnd(8+30+i, 5)

	for a in arrays:
		good.append(var2bytes(a))
	return all_variations