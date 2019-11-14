extends Node

const PoolArrays = preload("res://script/test_poolarrays.gd")
const BaseTypes = preload("res://script/test_basetypes.gd")
const Variator = preload("res://script/variator.gd")

var _tests = []
var _failed = true
var _idx = 0

# Called when the node enters the scene tree for the first time.
func _ready():
	COMPAT.set_physics_fps(480*2)

	var args = Array(OS.get_cmdline_args())
	var c_seed = OS.get_unix_time()
	var c_idx = null
	while args.size() > 0:
		var a = args[0]
		args.pop_front()
		if a == "--seed" and args.size() > 0:
			c_seed = int(args[0])
			args.pop_front()
		elif a == "--idx" and args.size() > 0:
			c_idx = int(args[0])
			args.pop_front()

	var good = []
	for t in PoolArrays.new().test(good, c_seed):
		_tests.append(t)

	for t in BaseTypes.new().test(good, NodePath(get_path()), c_seed):
		_tests.append(t)

	_failed = true
	print("Good: %d" % good.size())
	_failed = false

	COMPAT.enable_phys(self, true)
	if c_idx != null:
		var p = null
		while _idx <= c_idx:
			p = _tests.pop_back()
			_idx += 1
		print(p)
		get_tree().quit()
		COMPAT.enable_phys(self, false)
		if p != null:
			print(Array(p["original"]))
			print(Array(p["variation"]))
			var x = bytes2var(p["variation"])

func _physics_process(delta):
	_my_process(delta)

func _fixed_process(delta):
	_my_process(delta)

func _my_process(delta):
	if _failed:
		print("Failed")
		get_tree().quit()
		return
	if _tests.size() == 0:
		print("All good")
		get_tree().quit()
		return
	var p = _tests[_tests.size()-1]
	_tests.pop_back()

	printt("Testing:", _idx, p["seed"], p["notes"])
	var variation = p["variation"]
	_idx += 1

	# Anything after this might not work
	var x = bytes2var(variation)
	print(x)
	x = null