extends Control

# UDP abstractions, first to listen, then to send.
var udp_l = PacketPeerUDP.new()
var udp_s = PacketPeerUDP.new()
var uname

var TEXTEDIT_LIMIT = 35 #column/character limit
var current_text = ''
var cursor_line = 0
var cursor_column = 0

var CB_LIMIT = 14 #line/message limit
var cb_line = 0

func _ready():
	set_process_input(false)
	set_process(false)


func _on_T1_pressed():
	udp_l.listen(1025)
	udp_s.set_dest_address("127.0.0.1", 1026)
	uname = "Luiz"
	initialize()


func _on_T2_pressed():
	udp_l.listen(1026)
	udp_s.set_dest_address("127.0.0.1", 1025)
	uname = "Victor"
	initialize()

func initialize():
	$T1.hide()
	$T2.hide()
	$TextBGInner.show()
	$TextBGOuter.show()
	$ChatBox.show()	
	$TypeBox.show()
	$">".show()
	
	$TypeBox.grab_focus()
	
	set_process_input(true)
	set_process(true)

func _input(event):
	if event.is_action_pressed("send"):
		var msg = $TypeBox.get_line(0)
		$TypeBox.select_all()
		$TypeBox.cut()
		if msg == "":
			return
		# Se a mensagem não começar com brackets, converter a variavel para bytes (serializar), e enviar.
		if true:
			cb_insert_text(msg)
#			var msg_bytes = var2bytes(msg)
#			udp_s.put_packet(msg_bytes)
		# Caso contrario, enviar a mensagem em um PoolByteArray
		else:
			pass
		
#		udp_s.put_packet(PoolByteArray([20, 0, 0, 0, 0, 0, 0, 255]))

func _process(delta):
	if udp_l.get_available_packet_count() < 1:
		return
	var pkt = udp_l.get_packet()
	var msg = bytes2var(pkt)

func _on_TextEdit_text_changed():
	$TypeBox.cursor_set_line(0)

	var new_text = $TypeBox.text
	if new_text.length() > TEXTEDIT_LIMIT:
		$TypeBox.text = current_text
		# when replacing the text, the cursor will get moved to the beginning of the
		# text, so move it back to where it was 
		$TypeBox.cursor_set_column(cursor_column)

	else:
		current_text = $TypeBox.text
		# save current position of cursor for when we have reached the limit
		cursor_line = $TypeBox.cursor_get_line()
		cursor_column = $TypeBox.cursor_get_column()

func _on_TextEdit_cursor_changed():
	$TypeBox.cursor_set_line(0)

func cb_insert_text(message):
	if cb_line + 1 > CB_LIMIT:
		$ChatBox.select(0, 0, 0, TEXTEDIT_LIMIT)
		$ChatBox.cut()
		for i in range (1, CB_LIMIT):
#			print(str("i: ", i, ", contents: ", $ChatBox.get_line(i)))
			$ChatBox.select(i, 0, i, TEXTEDIT_LIMIT)
			var temp = $ChatBox.get_line(i)
			$ChatBox.cut()
			$ChatBox.cursor_set_line(i-1)
			$ChatBox.insert_text_at_cursor(temp)
		$ChatBox.cursor_set_line(CB_LIMIT - 1)
		$ChatBox.insert_text_at_cursor(message)
	else:
		$ChatBox.cursor_set_line(cb_line)
		$ChatBox.insert_text_at_cursor(str(uname, ": ", message, '\n'))
		cb_line += 1

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
