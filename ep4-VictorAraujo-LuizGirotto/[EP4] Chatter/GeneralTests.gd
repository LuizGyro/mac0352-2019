extends Control

func _ready():
#	var s = "hello"
#	var sb = var2bytes(s)
#	for c in sb:
#		print(c)
#	print(bytes2var(sb))

	var t = PoolByteArray([4, 0, 0, 0, 5, 0, 0, 0, 104, 101, 108, 108, 111])
	print(bytes2var(t))