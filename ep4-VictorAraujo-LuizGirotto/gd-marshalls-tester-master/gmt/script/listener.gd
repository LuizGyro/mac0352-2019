extends Node

var udp = PacketPeerUDP.new()
var _thread = Thread.new()
var _should_quit = false
var _valids = []
var _mutex = Mutex.new()

func my_process():
	if udp.get_available_packet_count() < 1:
		return
	var pkt = udp.get_packet()
	var x = bytes2var(pkt)
	if _mutex != null:
		_mutex.lock()
		_valids.append([pkt, x])
		_mutex.unlock()
	else:
		_valids.append(pkt)

func _exit_tree():
	_should_quit = true
	_thread.wait_to_finish()

func _process(delta):
	var node = get_node("Control/TextLabel")
	var text = node.get_text()
	if _mutex != null:
		_mutex.lock()
	for v in _valids:
		prints("Valid:", v[0], v[1])
		node.set_text(text + "Valid: %s %s\n" % v)
	if _mutex != null:
		_valids.clear()
	_mutex.unlock()

func _thread_func(userdata=null):
	while not _should_quit:
		OS.delay_msec(1)
		my_process()

# Called when the node enters the scene tree for the first time.
func _ready():
	udp.listen(1025)
	_thread.start(self, "_thread_func")
	print("Ready")
	COMPAT.enable_proc(self, true)