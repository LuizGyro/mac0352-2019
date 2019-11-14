extends Reference

var _data = COMPAT.ByteArr()
var _variations = []
var _seed = OS.get_unix_time()

func set_data(data, p_seed=null):
	self._data = COMPAT.ByteArr(Array(data))
	if p_seed != null:
		_seed = p_seed
	else:
		_seed = OS.get_unix_time()
	clear()
	add_variation(make_variation(), "Original, mutation seeds: %d" % _seed)
	seed(_seed)

func get_data():
	return _data

func get_variations():
	return _variations

func make_variation():
	var p = COMPAT.ByteArr()
	p.append_array(_data)
	return p

func add_variation(variation, notes=""):
	self._variations.append({"variation": variation, "notes": notes, "original": _data, "original_value": bytes2var(_data), "seed": _seed})

func variate_size(add, rem):
	for a in range(0, add):
		var p = make_variation()
		p.resize(p.size() + a)
		add_variation(p, "size + %d" % a)
	for r in range(0, rem):
		if _data.size() < r:
			continue
		var p = make_variation()
		p.resize(p.size() - r)
		add_variation(p, "size - %d" % r)

func variate_byte(idx, num):
	if _data.size() <= idx: return
	var p = make_variation()
	p.set(idx, num)
	add_variation(p, "byte[%d] = %d" % [idx, num])

func variate_byte_limit(idx, low=0, high=255):
	variate_byte(idx, low)
	variate_byte(idx, high)

func variate_byte_rnd(idx, num=1):
	for n in range(0, num):
		variate_byte(idx, (randi() % 255))

func variate_32(ofs=4, arr=[255, 255, 255, 127]):
	if ofs + 4 > _data.size():
		return
	var p = make_variation()
	for i in range(ofs, ofs+4):
		p.set(ofs + i, arr[i-ofs])
	add_variation(p, "4 bytes, ofs = %d, arr = %s" % [ofs, str(arr)])

func fuzzy_me():
	variate_size(2, 2)
	variate_byte_limit(randi()%_data.size())
	variate_byte_limit(randi()%_data.size())
	variate_byte_limit(randi()%_data.size())
	variate_32()
	for i in range(0, 3):
		variate_byte_rnd(1+i, 1)
		variate_byte_rnd(6+i, 5)
		variate_byte_limit(6+i, 0, 255)
	for i in range(0, 3):
		variate_byte_limit(8+30+i, 0, 255)
		variate_byte_rnd(8+30+i, 3)
		variate_byte_rnd(8+30+i, 5)
	variate_32(8)
	variate_32(_data.size()-4)

func clear():
	_variations.clear()