extends Node2D

# Should translate to 3 seconds
var colours = ["ff5c5c", "ffde5c", "65ff5c", "5cfbff", "695cff", "f95cff", "ffffff"]

const RETRY_AMNT = 3

var retry_counter = 0
var active = false

var mid_bob = false
var ascending = true

func _ready():
	set_physics_process(false)
#	set_process(true)
	
#func _process(delta):
#	if mid_bob:
#		return
#	mid_bob = true
#	if ascending:
#		$BobTween.interpolate_property(self, "position", Vector2(self.position.x, self.position.y), Vector2(self.position.x, self.position.y + 30), 0.8, Tween.TRANS_LINEAR, Tween.EASE_OUT_IN)
#		$BobTween.start()
#	else:
#		$BobTween.interpolate_property(self, "position", Vector2(self.position.x, self.position.y), Vector2(self.position.x, self.position.y - 30), 0.8, Tween.TRANS_LINEAR, Tween.EASE_OUT_IN)
#		$BobTween.start()

func _physics_process(delta):
	if retry_counter < RETRY_AMNT:
		retry_counter += delta
	else:
		retry_counter = 0
		#roll if is going to move
		randomize()
		if ((randi() % 10) < 8):
			return
		# return if tween is active
		if active:
			return
		else:
			active = true
			#define colour
			var clr_i = randi() % colours.size()
			self.set("modulate", Color(colours[clr_i]))
			# define side
			randomize()
			#begin left
			if (randi() % 2 < 1):
				$MoveTween.interpolate_property(self, "position", Vector2(-100, self.position.y), Vector2(1100, self.position.y), (randi() % 7) + 3, Tween.TRANS_LINEAR, Tween.EASE_OUT_IN)
				$MoveTween.start()
			#begin right
			else:
				$MoveTween.interpolate_property(self, "position", Vector2(1100, self.position.y), Vector2(-100, self.position.y), (randi() % 7) + 3, Tween.TRANS_LINEAR, Tween.EASE_OUT_IN)
				$MoveTween.start()
	

func _on_MoveTween_tween_completed( object, key ):
	active = false


#func _on_BobTween_tween_completed( object, key ):
#	if ascending:
#		ascending = false
#	else:
#		ascending = true
#	mid_bob = false
