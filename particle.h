/*
 *  CSCI 441, Computer Graphics, Fall 2019
 *
 *  Project: Midterm Project
 *  File: main.cpp
 *
 *	Author: Matthew Clough - Fall 2019
 *
 *  Description:
 *      Contains the abstract class for drawing a hero
 *
 */

// include the OpenGL library header
#ifdef __APPLE__					// if compiling on Mac OS
	#include <OpenGL/gl.h>
#else										// if compiling on Linux or Windows OS
	#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>	// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>				// for cos(), sin() functionality
#include <stdio.h>			// for printf functionality
#include <stdlib.h>			// for exit functionality
#include <time.h>			  // for time() functionality
#include <iostream>


#ifndef PARTICLE_H
#define PARTICLE_H
namespace PS{
	class Particle {
		public:
			glm::vec3 pos;
			glm::vec3 vel;
			int life = -1;


			Particle(glm::vec3 position, glm::vec3 velocity, int life);
			void draw();
			void update();

	};
}

PS::Particle::Particle(glm::vec3 position, glm::vec3 velocity, int l){
    pos = position;
	vel = velocity;
	life = l;
}

void PS::Particle::draw() {
    //cout<<"draw"<<endl;
}
void PS::Particle::update() {
	//cout<<"update"<<pos.x<<" "<<pos.y<<" "<<pos.z<<" "<<vel.x<<" "<<vel.y<<" "<<vel.z<<endl;
    pos.x = pos.x+vel.x;
	pos.y = pos.y+vel.y;
	pos.z = pos.z+vel.z;
	vel.y = vel.y-0.01;
	life--;
}

#endif
