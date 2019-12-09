Green Mario Guild - Lake David, Matt Clough, Joseph Kim

FP: The Grand (Re)Opening!

Description:	You are Green Mario and you are exploring Luigi(TM)'s mansion property. You can move you character around and explore with your trusty flashlight!
				If you reach the mansion's staircase entrance from the graveyard, you lose. No way to win. Only infinite loop.
Usage:
	run program: ./FP or make run
	movement: - WASD to control Green Mario.
			  - Left click + drag to move camera/Green Mario direction.
			  - 1 to switch between arc ball cam and first person cam.
	
Compliling:
	Type make in command line.
	
Implementation/Bugs:
	To use the map file we found we had to seperarate most of it's parts into individual obj files in order to properly render them from back to front.
	We also did not have enough time to do bounds for the whole map.
	We attempted to do md5 stuff for animation, but since we couldn't export our model in MD5 we opted to hack it with many obj files.
	
Input file format:
	No file inputs.
	
Responsibilities:
	Lake	: attempted MD5 loading + rendering in OpenGL, flashlight and moonlight, model animation
	Matt	: loading + texturing of map, rendering of map, movement, arc ball camera
	Joseph	: model animation, animation rendering in OpenGL, bounding boxes/height maps, first person camera
	
How long did this assignment take you?
	As a group 25 hrs

How much did the lab help you for this assignment? 1-10 (1 - did not help at all, 10 - this was exactly the same as the lab)
	Not much, kinda started with lab 11 but got rid of most of it. 5

How fun was this assignment? 1-10 (1 - discontinue this assignment, 10 - I wish I had more time to make it even better)
	Yes 8