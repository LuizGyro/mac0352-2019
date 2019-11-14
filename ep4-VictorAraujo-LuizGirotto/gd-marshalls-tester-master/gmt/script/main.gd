extends Control

func _ready():
	var args = Array(OS.get_cmdline_args())
	if args.size() > 1:
		if args[1] == "test":
			_on_Test_pressed()
		elif args[1] == "listen":
			_on_Listen_pressed()
		elif args[1] == "send":
			_on_Send_pressed()

func _on_Test_pressed():
	COMPAT.load_tester()

func _on_Listen_pressed():
	COMPAT.load_listener()

func _on_Send_pressed():
	COMPAT.load_sender()
