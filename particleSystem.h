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

#include <GL/glew.h>        // include GLEW to get our OpenGL 3.0+ bindings
#include <GLFW/glfw3.h>	// include GLFW framework header

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>				// for cos(), sin() functionality
#include <stdio.h>			// for printf functionality
#include <stdlib.h>			// for exit functionality
#include <time.h>			  // for time() functionality
#include <iostream>
#include <vector>

#include "particle.h"
using namespace std;


#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

namespace PS{
	class ParticleSystem {
		public:
			glm::vec3 source;
			float coneAngle, minv, maxv, width, depth, floor;
			int minl, maxl, spawnRate, NUM_POINTS;

			enum Type {fountain, rain};

			Type type;

			struct Vertex { GLfloat x, y, z; };

			vector<Particle*> points;

			GLuint pointsVAO, pointsVBO, alphaVBO;
			GLint vpos_attrib_location;
			GLint alpha_attrib_location;
			GLuint textureHandle;

			glm::vec3 eyePoint = glm::vec3( 10.0f, 10.0f, 10.0f );
			glm::vec3 lookAtPoint = glm::vec3( 0.0f,  0.0f,  0.0f );
			glm::mat4 modelMtx = glm::mat4(1.0f);

			ParticleSystem(GLint vpos_location, GLint alpha_location, GLuint texHandle, Type systemType, glm::vec3 position, float angle, float width, float depth, int minimumLife, int maximumLife, float minimumVelocity, float maximumVelocity, int spawningRate);
			void draw();
			void update(glm::mat4 mMtx, glm::vec3 ePoint, glm::vec3 laPoint);
		private:
			GLfloat randNumber( float max ) {
				return rand() / (GLfloat)RAND_MAX * max;
			}
			PS::Particle* genParticle();

	};
}

PS::ParticleSystem::ParticleSystem(GLint vpos_location, GLint alpha_location, GLuint texHandle, Type systemType, glm::vec3 position, float angle, float boxWidth, float fallDepth, int minimumLife, int maximumLife, float minimumVelocity, float maximumVelocity, int spawningRate){
	//cout<<"creaded particle system"<<endl;
	vpos_attrib_location = vpos_location;
	alpha_attrib_location = alpha_location;
	textureHandle = texHandle;
	source = position;
	coneAngle = angle;
	width = boxWidth;
	depth = fallDepth;
	minl = minimumLife;
	maxl = maximumLife;
	minv = minimumVelocity;
	maxv = maximumVelocity;
	spawnRate = spawningRate;
	type = systemType;
	floor = source.y-depth;

	NUM_POINTS = maxl*spawnRate;

	for( int i = 0; i < NUM_POINTS; i++ ) {
		Particle* p = new PS::Particle(source, glm::vec3(0.0f,0.0f,0.0f), -1);
		points.push_back(p);
	}

	//cout<<points.size()<<endl;

	glGenVertexArrays( 1, &pointsVAO );
	glBindVertexArray( pointsVAO );

	glGenBuffers( 1, &pointsVBO );
	glBindBuffer( GL_ARRAY_BUFFER, pointsVBO );
	glBufferData( GL_ARRAY_BUFFER, points.size()*sizeof(Vertex), NULL, GL_STATIC_DRAW );
	glEnableVertexAttribArray( vpos_attrib_location );
	glVertexAttribPointer( vpos_attrib_location, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	glGenBuffers( 1, &alphaVBO );
	glBindBuffer( GL_ARRAY_BUFFER, alphaVBO );
	glBufferData( GL_ARRAY_BUFFER, points.size()*sizeof(float), NULL, GL_STATIC_DRAW );
	glEnableVertexAttribArray( alpha_attrib_location );
	glVertexAttribPointer( alpha_attrib_location, 1, GL_FLOAT, GL_FALSE, 0, (void*)0 );
}

PS::Particle* PS::ParticleSystem::genParticle(){
	//cout<<"generating particle"<<endl;
	float vel = minv + randNumber(maxv-minv);
	glm::vec3 velocity;
	int life;
	glm::vec3 spawnLocation;
	
	if(type == Type::fountain){
		life = minl + randNumber(maxl-minl);
		GLfloat direction = randNumber(2.0f*M_PI);
		GLfloat angle = randNumber(coneAngle);
    	float r = sinf(angle);
		velocity = vel*glm::normalize(glm::vec3(1*r*sinf(direction), cosf(angle), -1*r*cosf(direction)));
		spawnLocation = source;
	}else if(type == Type::rain){
		life = 0;
		float x = source.x-width/2+randNumber(width);
		float z = source.z-width/2+randNumber(width);
		velocity = glm::vec3(0.0f, -vel, 0.0f);
		spawnLocation = glm::vec3(x, source.y, z);
	}
	
	//cout<<velocity.x<<velocity.y<<velocity.z<<endl;
	Particle* p = new PS::Particle(spawnLocation, velocity, life);
	return p;
	
}

void PS::ParticleSystem::draw() {
    // TODO #1 : sort!
	glm::vec3 v = glm::normalize(eyePoint-lookAtPoint);
	int order[NUM_POINTS];
	double dists[NUM_POINTS];
	

	

	for(int i = 0;i<NUM_POINTS;i++){
		glm::vec4 p = glm::vec4(points[i]->pos.x, points[i]->pos.y, points[i]->pos.z, 0) * modelMtx;
		glm::vec3 ep = eyePoint-glm::vec3(p.x, p.y, p.z);
		double dist = glm::dot(ep, v);
		order[i] = i;
		dists[i]= dist;
	}

	for(int i =0; i<NUM_POINTS-1; i++){
		for(int j = i; j<NUM_POINTS; j++){
			if(dists[i]<dists[j]){
				int oi = order[i];
				double di = dists[i];
				dists[i] = dists[j];
				order[i] = order[j];
				dists[j] = di;
				order[j] = oi;
			}
		}
	}

	Vertex verts[NUM_POINTS];
	float alphas[NUM_POINTS];
	for( unsigned int i = 0; i< NUM_POINTS; i++){
		verts[i].x = points[order[i]]->pos.x;
		verts[i].y = points[order[i]]->pos.y;
		verts[i].z = points[order[i]]->pos.z;
		if(type == Type::fountain){
			if(points[order[i]]->life<0){
				alphas[i] = 0.0;
			}else{
				alphas[i] = (float)(points[order[i]]->life);
			}
		}else if(type == Type::rain){
			if(points[order[i]]->pos.y<floor){
				alphas[i] = 0.0;
			}else{
				alphas[i] = points[order[i]]->pos.y-floor;
			}
		}
		
		
		//alphas[i] = randNumber(1.0);
		//cout<<alphas[i]<<" ";
	}
	//cout<<endl;
	//cout<<sizeof(verts)/sizeof(verts[0])<<endl;
	//for( unsigned int i = 0; i<NUM_POINTS; i++){
	//	cout<<i<<" "<<verts[i].x<<" "<<verts[i].y<<" "<<verts[i].z<<endl;
	//}
	glBindVertexArray( pointsVAO );
	// TODO #2 : send our sorted data
	glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

	glBindBuffer(GL_ARRAY_BUFFER, alphaVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(alphas), alphas);
	// LOOKHERE #4
	glBindTexture( GL_TEXTURE_2D, textureHandle );
	glDrawArrays( GL_POINTS, 0, NUM_POINTS );
}
void PS::ParticleSystem::update(glm::mat4 mMtx, glm::vec3 ePoint, glm::vec3 laPoint) {
    modelMtx = mMtx;
	eyePoint = ePoint;
	lookAtPoint = laPoint;

	for(int i = 0;i<NUM_POINTS;i++){
		points[i]->update();
		//cout<<"life: "<<points[i]->life<<endl;
	}

	if(type == Type::fountain){
		for(int i=0; i<spawnRate;i++){
			int index = 0;
			int lowest = maxl;
			
			for(int j = 0;j<NUM_POINTS;j++){
				if(points[j]->life<lowest){
					index = j;
					lowest = points[j]->life;
				}
				if(lowest<=0){
					break;
				}
			}
			//cout<<"lowest: "<<lowest<<" "<<index<<endl;
			delete points[index];
			points[index] = genParticle();
		}
	}else if(type == Type::rain){
		for(int i=0; i<spawnRate;i++){
			int index = 0;
			int lowest = source.y;
			
			for(int j = 0;j<NUM_POINTS;j++){
				if(points[j]->pos.y<lowest){
					index = j;
					lowest = points[j]->pos.y;
				}
				if(lowest<=floor){
					break;
				}
			}
			//cout<<"lowest: "<<lowest<<" "<<index<<endl;
			delete points[index];
			points[index] = genParticle();
		}
	}

	
}

#endif
