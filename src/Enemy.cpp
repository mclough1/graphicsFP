#include "../include/Enemy.h"

#include <glm/gtc/matrix_transform.hpp>

#include <CSCI441/objects3.hpp>

#include <math.h>
#include <stdlib.h>

Enemy::Enemy() {
    _radius = 0.5;
    location = glm::vec3(0,0,0);
    direction = glm::vec3(1,0,0);
    _rotation = 3.14f/2.0f;
    velocity = 0.1;
    _color = glm::vec4(	rand() % 50 / 100.0 + 0.5, 
						rand() % 50 / 100.0 + 0.5, 
						rand() % 50 / 100.0 + 0.5,
						1.0f );
    state = State::ALIVE;
}

Enemy::Enemy( glm::vec3 l, glm::vec3 d, double r ) : location(l), direction(d) {
    _rotation = 3.14f/2.0f;
    _color = glm::vec4(rand() % 50 / 100.0 + 0.5, 
					   rand() % 50 / 100.0 + 0.5, 
					   rand() % 50 / 100.0 + 0.5,
					   1.0f);
    _radius = 1.0;
    velocity = 0.1;
    state = State::ALIVE;
}

void Enemy::draw( glm::mat4 modelMtx, GLint uniform_modelMtx_loc, GLint uniform_color_loc ) const {

    glm::vec3 directionNormalized = glm::normalize( direction );
    glm::vec3 rotationAxis = glm::cross( directionNormalized, glm::vec3(0,1,0) );

    modelMtx = glm::translate( modelMtx, location );
    modelMtx = glm::translate( modelMtx, glm::vec3( 0, _radius, 0 ) );
    
    glUniformMatrix4fv( uniform_modelMtx_loc, 1, GL_FALSE, &modelMtx[0][0] );
    glUniform4fv( uniform_color_loc, 1, &_color[0] );

    CSCI441::drawSolidSphere( _radius, 16, 16 );
    //CSCI441::drawSolidCube( 2.0 );
    modelMtx = glm::translate( modelMtx, glm::vec3( 0, _radius/2, 0 )+0.5f*directionNormalized );
    modelMtx = glm::rotate( modelMtx, -(float)_rotation, rotationAxis );
    glUniformMatrix4fv( uniform_modelMtx_loc, 1, GL_FALSE, &modelMtx[0][0] );
    glUniform4fv( uniform_color_loc, 1, &_color[0] );
    CSCI441::drawSolidCone( _radius/10, _radius, 16, 16 );
}

void Enemy::moveForward(glm::vec3 target) {
    if(state == State::ALIVE){
        glm::vec3 pull = normalize(target-location);
        float acceleration = glm::dot(direction, pull);
        velocity += 0.005*acceleration;
        if(velocity < 0){
            velocity = 0.0f;
        }
        direction = normalize(direction + pull/8.0f);

        location += direction*velocity;
    }else if(state == State::DEAD){
        
        if(location.y>-50){
            location.y-=0.1;
        }
    }
    
    
    //_rotation -= 0.1;
    //if( _rotation < 0 ) {
    //    _rotation += 6.28;
    //}
}

void Enemy::moveBackward() {
    location -= direction*velocity;
    //_rotation += 0.1;
    //if( _rotation > 6.28 ) {
    //    _rotation -= 6.28;
    //}
}

double Enemy::getRadius() const {
	return _radius;
}