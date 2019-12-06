Matthew Clough, mclough@mymail.mines.edu
Assignment 6

Compiling:
Might need to change the paths in the makefile, but just run the make command and run the executable with a valid path to a config file

Controls:

W/S move foreward/backward
leftclick and drag to turn camera


Config Format:
Config file is for only the particle systems, the systems are read one line for one system, data elements separated by spaces

	fountain:
		F sourceX(float) sourceY(float) sourceZ(float) ConeAngle(float) minHealth(int) maxHealth(int) minVelocity(float) maxVelocity(float) spawnRate(int)
	rain:
		R sourceX(float) sourceY(float) sourceZ(float) width(float) depth(float) minVelocity(float) maxVelocity(float) spawnRate(int)

You can use multiple particle systems, however since each system sorts its own particles, the particles of other systems are not in order, so theressome overlapping issues


This took me at least 16 hours
Lab contributed quite a bit 8
8 on fun as well I did start late due to some other assignments that take precedence but I also wish I had more time (I say as I turn it in 2 days late)