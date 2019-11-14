extends Node

var udp = PacketPeerUDP.new()
var MAX_SIZE = 20
var PERMUTATIONS = 20

var _type = -1
var _size = 0
var _permutation = 0
var _buffer = StreamPeerBuffer.new()
var _seed = OS.get_unix_time()
var _thread = Thread.new()
var _shuold_quit = false

# Called when the node enters the scene tree for the first time.
func _ready():
	print("Seed: %d" % _seed)
	seed(_seed)
	COMPAT.set_dest_address(udp, "127.0.0.1", 1025)
	_thread.start(self, "_thread_func")
	COMPAT.enable_proc(self, true)
	pass # Replace with function body.

func _process(delta):
	if _shuold_quit:
		_thread.wait_to_finish()
		get_tree().quit()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _thread_func(userdata=null):
	while not _shuold_quit:
		OS.delay_msec(5)
		_send()

func _send():
	if _type >= TYPE_MAX+1:
		_shuold_quit = true
		return
	if _size > MAX_SIZE:
		_size = 0
		_permutation = 0
		_type += 1
		return
	if _permutation > PERMUTATIONS:
		_permutation = 0
		_size += 1
		return
	_buffer.resize(_size + 4)
	_buffer.seek(0)
	_buffer.put_u32(_type)
	for i in range(0, _size):
		_buffer.put_u8(randi() % 255)
	print(Array(_buffer.get_data_array()))
	udp.put_packet(_buffer.get_data_array())
	_permutation += 1
