extends Control

# UDP abstractions, first to listen, then to send.
var udp_l = PacketPeerUDP.new()
var udp_s = PacketPeerUDP.new()

var TEXTEDIT_LIMIT = 35
var current_text = ''
var cursor_line = 0
var cursor_column = 0

func _ready():
	set_process_input(false)
	set_process(false)


func _on_T1_pressed():
	udp_l.listen(1025)
	udp_s.set_dest_address("127.0.0.1", 1026)
	initialize()


func _on_T2_pressed():
	udp_l.listen(1026)
	udp_s.set_dest_address("127.0.0.1", 1025)
	initialize()

func initialize():
	$T1.hide()
	$T2.hide()
	$TextBGInner.show()
	$TextBGOuter.show()
	$"TextEdit".show()
	$">".show()
	
	$"TextEdit".grab_focus()
	
	set_process_input(true)
	set_process(true)

func _input(event):
	if event.is_action_pressed("send"):
		var msg = $TextEdit.get_line(0)
		$TextEdit.select_all()
		$TextEdit.cut()
		print(msg)
#		udp_s.put_packet(PoolByteArray([20, 0, 0, 0, 0, 0, 0, 255]))

func _process(delta):
	if udp_l.get_available_packet_count() < 1:
		return
	var pkt = udp_l.get_packet()
	var msg = bytes2var(pkt)

func _on_TextEdit_text_changed():
	$TextEdit.cursor_set_line(0)

	var new_text = $TextEdit.text
	if new_text.length() > TEXTEDIT_LIMIT:
		$TextEdit.text = current_text
		# when replacing the text, the cursor will get moved to the beginning of the
		# text, so move it back to where it was 
		$TextEdit.cursor_set_column(cursor_column)

	else:
		current_text = $TextEdit.text
		# save current position of cursor for when we have reached the limit
		cursor_line = $TextEdit.cursor_get_line()
		cursor_column = $TextEdit.cursor_get_column()

func _on_TextEdit_cursor_changed():
	$TextEdit.cursor_set_line(0)


# TEST

func _on_Button_pressed():
#	var a = PoolByteArray([20, 0, 0, 0, 0, 0, 0, 255])
#	var m = var2bytes(a)
#	for n in m:
#		print(n)
#	bytes2var(m)

#	var message = PoolByteArray([20, 0, 0, 0, 0, 0, 0, 255])
#	$TextEdit.insert_text_at_cursor(message.get_string_from_ascii())
#	print(message.get_string_from_ascii())
	bytes2var(PoolByteArray([20, 0, 0, 0, 0, 0, 0, 255]))

#	bytes2var([20, 0, 0, 0, 0, 0, 0, 255])
